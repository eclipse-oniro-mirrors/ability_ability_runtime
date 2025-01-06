/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "ability_record.h"
#include "isession_handler_interface.h"

namespace OHOS {
namespace AAFwk {
class SessionInfo;
class StatusBarDelegateManager;
struct AbilityRunningInfo;
struct MissionValidResult;

struct SpecifiedRequest {
    int32_t requestId = 0;
    AbilityRequest abilityRequest;
    bool preCreateProcessName = false;
    bool isCold = false;
    int32_t persistentId = 0;
    uint32_t sceneFlag = 0;
    uint32_t callingTokenId = 0;

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
     * @param isColdStart the session info of the ability is or not cold start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo, uint32_t sceneFlag,
        bool &isColdStart);

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
        const sptr<IRemoteObject> &token, const AppExecFwk::WindowConfig &windowConfig);

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
    int MinimizeUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser, uint32_t sceneFlag);

    /**
     * GetUIAbilityRecordBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetUIAbilityRecordBySessionInfo(const sptr<SessionInfo> &sessionInfo);

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
    int CloseUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant, bool isClearSession);

    /**
     * Set rootSceneSession by SCB.
     *
     * @param rootSceneSession Indicates root scene session of SCB.
     */
    void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession);

    int NotifySCBToStartUIAbility(AbilityRequest &abilityRequest);

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
    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * resolve the call ipc of ability for scheduling oncall.
     *
     * @param abilityRequest target ability request.
     */
    int ResolveLocked(const AbilityRequest &abilityRequest);

    /**
     * Call UIAbility by SCB.
     *
     * @param sessionInfo the session info of the ability to be called.
     * @param isColdStart the session of the ability is or not cold start.
     */
    void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isColdStart);

    /**
     * OnAcceptWantResponse.
     *
     * @param want the want of the ability to start.
     * @param abilityRequest the flag of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId = 0);

    /**
     * OnStartSpecifiedProcessResponse.
     *
     * @param want the want of the ability to start.
     * @param abilityRequest target ability request.
     */
    void OnStartSpecifiedProcessResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId = 0);

    /**
     * OnStartSpecifiedAbilityTimeoutResponse.
     *
     * @param want the want of the ability to start.
     */
    void OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want, int32_t requestId = 0);

    /**
     * OnStartSpecifiedProcessTimeoutResponse.
     *
     * @param want the want of the ability to start.
     */
    void OnStartSpecifiedProcessTimeoutResponse(const AAFwk::Want &want, int32_t requestId = 0);

    /**
     * Start specified ability by SCB.
     *
     * @param want Want information.
     */
    void StartSpecifiedAbilityBySCB(const Want &want);

    /**
     * CallRequestDone, after invoke callRequest, ability will call this interface to return callee.
     *
     * @param abilityRecord ability's record.
     * @param callStub ability's callee.
     */
    void CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord, const sptr<IRemoteObject> &callStub);

    /**
     * release the connection of this call.
     *
     * @param connect caller callback ipc.
     * @param element target ability name.
     */
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

    bool PrepareTerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void SetSessionHandler(const sptr<ISessionHandler> &handler);

    /**
     * Get abilityRecord by session id.
     *
     * @param sessionId the session id.
     * @return Returns abilityRecord on success, nullptr on failure.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordsById(int32_t sessionId) const;

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

    bool IsCallerInStatusBar();

    bool IsInStatusBar(uint32_t accessTokenId);

    int32_t TryPrepareTerminateByPids(const std::vector<int32_t>& pids);

    int ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow);

    int ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow);

    std::vector<std::shared_ptr<AbilityRecord>> GetAbilityRecordsByName(const AppExecFwk::ElementName &element);

    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token) const;

#ifdef SUPPORT_GRAPHICS
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &token);
#endif
    void CompleteFirstFrameDrawing(int32_t sessionId) const;

    int32_t GetAbilityStateByPersistentId(int32_t persistentId, bool &state);

    void NotifySCBToHandleAtomicServiceException(sptr<SessionInfo> sessionInfo, int32_t errorCode,
        const std::string& errorReason);

    int32_t CleanUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void EnableListForSCBRecovery();

private:
    int32_t GetPersistentIdByAbilityRequest(const AbilityRequest &abilityRequest, bool &reuse) const;
    int32_t GetReusedSpecifiedPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;
    int32_t GetReusedStandardPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;
    int32_t GetReusedCollaboratorPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;
    std::string GenerateProcessNameForNewProcessMode(const AppExecFwk::AbilityInfo& abilityInfo);
    void PreCreateProcessName(AbilityRequest &abilityRequest);
    void UpdateProcessName(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &abilityRecord);
    void UpdateAbilityRecordLaunchReason(const AbilityRequest &abilityRequest,
        std::shared_ptr<AbilityRecord> &abilityRecord) const;
    void EraseAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, bool success,
        AbilityState state = AbilityState::INITIAL);
    void CompleteForegroundSuccess(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void HandleForegroundFailed(const std::shared_ptr<AbilityRecord> &ability,
        AbilityState state = AbilityState::INITIAL);
    void HandleForegroundTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void NotifySCBToHandleException(const std::shared_ptr<AbilityRecord> &ability, int32_t errorCode,
        const std::string& errorReason);
    void MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void PrintTimeOutLog(std::shared_ptr<AbilityRecord> ability, uint32_t msgId, bool isHalf = false);
    void DelayCompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminateLocked(const std::shared_ptr<AbilityRecord> &abilityRecord);
    bool IsContainsAbilityInner(const sptr<IRemoteObject> &token) const;
    bool CheckProperties(const std::shared_ptr<AbilityRecord> &abilityRecord, const AbilityRequest &abilityRequest,
        AppExecFwk::LaunchMode launchMode) const;
    void NotifyAbilityToken(const sptr<IRemoteObject> &token, const AbilityRequest &abilityRequest) const;
    void PrepareCloseUIAbility(std::shared_ptr<AbilityRecord> abilityRecord,
        int resultCode, const Want *resultWant, bool isClearSession);
    int CloseUIAbilityInner(std::shared_ptr<AbilityRecord> abilityRecord);
    int32_t BackToCallerAbilityWithResultLocked(sptr<SessionInfo> currentSessionInfo,
        std::shared_ptr<AbilityRecord> callerAbilityRecord);

    // byCall
    int CallAbilityLocked(const AbilityRequest &abilityRequest);
    sptr<SessionInfo> CreateSessionInfo(const AbilityRequest &abilityRequest) const;
    int NotifySCBPendingActivation(sptr<SessionInfo> &sessionInfo, const AbilityRequest &abilityRequest);
    int ResolveAbility(const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest) const;
    std::vector<std::shared_ptr<AbilityRecord>> GetAbilityRecordsByNameInner(const AppExecFwk::ElementName &element);
    void HandleForegroundCollaborate(const AbilityRequest &abilityRequest,
        std::shared_ptr<AbilityRecord> abilityRecord);

    void NotifyStartSpecifiedAbility(AbilityRequest &request, const AAFwk::Want &want);
    void NotifyRestartSpecifiedAbility(const AbilityRequest &request, const sptr<IRemoteObject> &token);
    int MoveAbilityToFront(const AbilityRequest &abilityRequest, const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr);
    int SendSessionInfoToSCB(std::shared_ptr<AbilityRecord> &callerAbility, sptr<SessionInfo> &sessionInfo);
    int StartAbilityBySpecifed(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &callerAbility,
        int32_t requestId);

    void SetLastExitReason(std::shared_ptr<AbilityRecord> &abilityRecord) const;
    void SetReceiverInfo(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &abilityRecord) const;

    bool CheckPrepareTerminateEnable(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * @brief Execute PrepareTerminateApp when it is implemented
     *
     * @param pid process id
     * @param tokens the tokens of ability records
     * @return Returns the tokens that still need to execute PrepareTerminate.
     */
    std::vector<sptr<IRemoteObject>> PrepareTerminateAppAndGetRemaining(
        int32_t pid, std::vector<sptr<IRemoteObject>> tokens);

    bool GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const;

    bool CheckSessionInfo(sptr<SessionInfo> sessionInfo) const;
    std::shared_ptr<AbilityRecord> CreateAbilityRecord(AbilityRequest &abilityRequest,
        sptr<SessionInfo> sessionInfo) const;
    void AddCallerRecord(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        std::shared_ptr<AbilityRecord> uiAbilityRecord) const;
    void CheckSpecified(int32_t requestId, std::shared_ptr<AbilityRecord> uiAbilityRecord);
    void SendKeyEvent(const AbilityRequest &abilityRequest) const;
    bool CheckPid(const std::shared_ptr<AbilityRecord> abilityRecord, const int32_t pid) const;
    std::shared_ptr<StatusBarDelegateManager> GetStatusBarDelegateManager();
    int32_t DoProcessAttachment(std::shared_ptr<AbilityRecord> abilityRecord);
    void BatchCloseUIAbility(const std::unordered_set<std::shared_ptr<AbilityRecord>>& abilitySet);
    void TerminateSession(std::shared_ptr<AbilityRecord> abilityRecord);
    int StartWithPersistentIdByDistributed(const AbilityRequest &abilityRequest, int32_t persistentId);
    void CheckCallerFromBackground(std::shared_ptr<AbilityRecord> callerAbility, sptr<SessionInfo> &sessionInfo);
    int32_t DoCallerProcessAttachment(std::shared_ptr<AbilityRecord> abilityRecord);
    std::shared_ptr<AbilityRecord> GenerateAbilityRecord(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        bool &isColdStart);
    std::shared_ptr<AbilityRecord> FindRecordFromTmpMap(const AbilityRequest &abilityRequest);
    void PostCallTimeoutTask(std::shared_ptr<AbilityRecord> abilityRecord);
    bool AddStartCallerTimestamp(int32_t callerUid);
    std::shared_ptr<AbilityRecord> FindRecordFromSessionMap(const AbilityRequest &abilityRequest);
    bool HasAbilityRequest(const AbilityRequest &abilityRequest);
    void AddAbilityRequest(const AbilityRequest &abilityRequest, int32_t requestId);
    void RemoveAbilityRequest(int32_t requestId);
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
    bool HasAppRecord(const AbilityRequest &abilityRequest);
    bool HandleStartSpecifiedCold(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo, uint32_t sceneFlag);
    bool HandleColdAcceptWantDone(const AAFwk::Want &want, const std::string &flag,
        const SpecifiedRequest &specifiedRequest);
    void HandleLegacyAcceptWantDone(AbilityRequest &abilityRequest, int32_t requestId,
        const std::string &flag, const AAFwk::Want &want);
    std::shared_ptr<SpecifiedRequest> GetSpecifiedRequest(int32_t requestId);
    void PutSpecifiedFlag(int32_t requestId, const std::string &flag);

    int32_t userId_ = -1;
    mutable ffrt::mutex sessionLock_;
    std::unordered_map<int32_t, std::shared_ptr<AbilityRecord>> sessionAbilityMap_;
    std::unordered_map<int64_t, std::shared_ptr<AbilityRecord>> tmpAbilityMap_;
    std::unordered_map<std::shared_ptr<AbilityRecord>, std::list<AbilityRequest>> callRequestCache_;
    std::list<std::shared_ptr<AbilityRecord>> terminateAbilityList_;
    sptr<IRemoteObject> rootSceneSession_;
    int32_t requestId_ = 0;
    std::map<int32_t, AbilityRequest> specifiedRequestMap_;
    sptr<ISessionHandler> handler_;
    ffrt::mutex statusBarDelegateManagerLock_;
    std::shared_ptr<StatusBarDelegateManager> statusBarDelegateManager_;
    bool isSCBRecovery_ = false;
    std::unordered_set<int32_t> coldStartInSCBRecovery_;

    ffrt::mutex startUIAbilityCallerTimestampsLock_;
    std::map<int32_t, std::vector<int64_t>> startUIAbilityCallerTimestamps_;
    std::map<int32_t, std::shared_ptr<AbilityRequest>> startAbilityCheckMap_;

    std::map<std::string, std::list<std::shared_ptr<SpecifiedRequest>>> specifiedRequestList_;
    std::map<int32_t, std::string> specifiedFlagMap_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H