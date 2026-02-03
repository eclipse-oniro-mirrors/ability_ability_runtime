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

#include "ability_manager_constants.h"
#include "ffrt.h"
#include "isession_handler_interface.h"
#include "ui_ability_record.h"

namespace OHOS {
namespace Rosen {
struct PendingSessionActivationConfig;
}
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
    int32_t requestListId = -1;
    uint32_t sceneFlag = 0;
    uint32_t callingTokenId = 0;
    AbilityRequest abilityRequest;

    SpecifiedRequest(int32_t requestId, AbilityRequest request) : requestId(requestId), abilityRequest(request) {}
};

struct AbilitiesRequest {
    int32_t requestListId = -1;
    std::string requestKey;
    std::vector<std::pair<int32_t, sptr<SessionInfo>>> sessionInfoList;
    uint32_t doneCount = 0;
    sptr<IRemoteObject> callerToken;
};

class UIAbilityLifecycleManager : public std::enable_shared_from_this<UIAbilityLifecycleManager> {
public:
    UIAbilityLifecycleManager() = default;
    explicit UIAbilityLifecycleManager(int32_t userId);
    virtual ~UIAbilityLifecycleManager() = default;

    void SignRestartAppFlag(int32_t uid, const std::string &instanceKey, bool isAppRecovery = false);
    void SignRestartProcess(int32_t pid);

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

    /**
     * Report grant uri permission event.
     *
     * @param abilityRequest ability request info.
     * @param callScenario uri grant method.
     */
    void ReportGrantUriPermissionEvent(const AbilityRequest& abilityRequest, const std::string& callScenario);

    /**
     * Triggered when app state changed.
     *
     * @param info appInfo on changed.
     */
    void OnAppStateChanged(const AppInfo &info);

    void HandleTerminatedOrEndState(const AppInfo &info);

    void HandleColdStartState(const AppInfo &info);

    void HandleOtherAppState(const AppInfo &info);

    bool IsMatchingAppInfo(const AppInfo &info, const UIAbilityRecordPtr &abilityRecord);

    /**
     * Triggered when app uninstall.
     *
     * @param bundleName application bundle name.
     * @param uid application uid.
     */
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

    /**
     * @brief Register status bar delegate
     * @param delegate The status bar delegate to register
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate);

    /**
     * @brief Check if caller is in status bar
     * @param instanceKey The instance key to check
     * @return true if caller is in status bar, false otherwise
     */
    bool IsCallerInStatusBar(const std::string &instanceKey);

    /**
     * @brief Check if access token is for status bar
     * @param accessTokenId The access token ID to check
     * @param isMultiInstance Whether it's a multi-instance scenario
     * @return true if access token is for status bar, false otherwise
     */
    bool IsInStatusBar(uint32_t accessTokenId, bool isMultiInstance);

    /**
     * @brief Check if status bar is supported
     * @return true if status bar is supported, false otherwise
     */
    bool IsSupportStatusBar();

    /**
     * @brief Try to prepare terminate abilities by process IDs
     * @param pids List of process IDs to terminate
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t TryPrepareTerminateByPids(const std::vector<int32_t>& pids, bool clear);

    /**
     * @brief Change ability visibility
     * @param token Ability token
     * @param isShow Whether to show the ability
     * @return ERR_OK if successful, error code otherwise
     */
    int ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow);

    /**
     * @brief Change UI ability visibility through SCB
     * @param sessionInfo Session information
     * @param isShow Whether to show the ability
     * @return ERR_OK if successful, error code otherwise
     */
    int ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow);

    /**
     * @brief Get ability records by element name
     * @param element The element name to search for
     * @return Vector of matching ability records
     */
    std::vector<UIAbilityRecordPtr> GetAbilityRecordsByName(const AppExecFwk::ElementName &element,
        int32_t appIndex);

    /**
     * @brief Get ability record by token
     * @param token The ability token
     * @return Shared pointer to the ability record, nullptr if not found
     */
    UIAbilityRecordPtr GetAbilityRecordByToken(const sptr<IRemoteObject> &token) const;

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Complete first frame drawing notification
     * @param token The ability token
     */
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &token);
#endif
    /**
     * @brief Complete first frame drawing notification by session ID
     * @param sessionId The session ID
     */
    void CompleteFirstFrameDrawing(int32_t sessionId) const;

    /**
     * @brief Get ability state by persistent ID
     * @param persistentId The persistent ID to check
     * @param state Output parameter for the state
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t GetAbilityStateByPersistentId(int32_t persistentId, bool &state);

    /**
     * @brief Notify SCB to handle atomic service exception
     * @param sessionInfo The session information
     * @param errorCode The error code
     * @param errorReason The error reason description
     */
    void NotifySCBToHandleAtomicServiceException(sptr<SessionInfo> sessionInfo, int32_t errorCode,
        const std::string& errorReason);

    /**
     * @brief Clean up UI ability resources
     * @param abilityRecord The ability record to clean up
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t CleanUIAbility(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Enable list for SCB recovery
     */
    void EnableListForSCBRecovery();

    /**
     * @brief Set kill flag for permission update
     * @param accessTokenId The access token ID
     */
    void SetKillForPermissionUpdateFlag(uint32_t accessTokenId);

    /**
     * @brief Handle prepare terminate ability completion
     * @param abilityRecord The ability record
     * @param isTerminate Whether termination was completed
     */
    void PrepareTerminateAbilityDone(UIAbilityRecordPtr abilityRecord, bool isTerminate);

    /**
     * @brief Handle try prepare terminate by PIDs completion
     * @param moduleName The module name
     * @param prepareTermination The termination state
     * @param isExist Whether the module exists
     */
    void TryPrepareTerminateByPidsDone(const std::string &moduleName, int32_t prepareTermination, bool isExist);
    
    /**
     * @brief Revoke delegator
     * @param token The token to revoke
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t RevokeDelegator(sptr<IRemoteObject> token);

    /**
     * @brief Check if bundle is starting
     * @param pid The process ID to check
     * @return true if bundle is starting, false otherwise
     */
    bool IsBundleStarting(pid_t pid);

    /**
     * @brief Record PID killing information
     * @param pid The process ID
     * @param reason The reason for killing
     * @param isKillPrecedeStart Whether kill precedes start
     */
    void RecordPidKilling(pid_t pid, const std::string &reason, bool isKillPrecedeStart);

    /**
     * @brief Notify SCB about startup exception
     * @param requestId The request ID
     * @param reason The exception reason
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t NotifyStartupExceptionBySCB(int32_t requestId, const std::string &reason);

    ErrCode IsUIAbilityAlreadyExist(const Want &want, const std::string &specifiedFlag,
        int32_t appIndex, const std::string &instanceKey, AppExecFwk::LaunchMode launchMode);

    void HandleUIAbilityDiedByPid(pid_t pid);

private:
    /**
     * @brief Add starting process ID to tracking list
     * @param pid The process ID to add
     */
    void AddStartingPid(pid_t pid);

    /**
     * @brief Remove starting process ID from tracking list
     * @param pid The process ID to remove
     */
    void RemoveStartingPid(pid_t pid);

    /**
     * @brief Mark starting flag in ability request
     * @param abilityRequest The ability request to modify
     */
    void MarkStartingFlag(const AbilityRequest &abilityRequest);

    /**
     * @brief Get persistent ID from ability request
     * @param abilityRequest The ability request to check
     * @param reuse Output parameter indicating if ID should be reused
     * @return The persistent ID
     */
    int32_t GetPersistentIdByAbilityRequest(const AbilityRequest &abilityRequest, bool &reuse) const;

    /**
     * @brief Get reused specified persistent ID
     * @param abilityRequest The ability request to check
     * @param reuse Output parameter indicating if ID should be reused
     * @return The persistent ID
     */
    int32_t GetReusedSpecifiedPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;

    /**
     * @brief Get reused standard persistent ID
     * @param abilityRequest The ability request to check
     * @param reuse Output parameter indicating if ID should be reused
     * @return The persistent ID
     */
    int32_t GetReusedStandardPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;

    /**
     * @brief Get reused collaborator persistent ID
     * @param abilityRequest The ability request to check
     * @param reuse Output parameter indicating if ID should be reused
     * @return The persistent ID
     */
    int32_t GetReusedCollaboratorPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;

    /**
     * @brief Generate process name for new process mode
     * @param abilityInfo The ability info to generate name for
     * @return The generated process name
     */
    std::string GenerateProcessNameForNewProcessMode(const AppExecFwk::AbilityInfo& abilityInfo);

    /**
     * @brief Pre-create process name in ability request
     * @param abilityRequest The ability request to modify
     */
    void PreCreateProcessName(AbilityRequest &abilityRequest);

    /**
     * @brief Update process name in ability record
     * @param abilityRequest The ability request containing new name
     * @param abilityRecord The ability record to update
     */
    void UpdateProcessName(const AbilityRequest &abilityRequest, UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Update ability record launch reason
     * @param abilityRequest The ability request containing launch info
     * @param abilityRecord The ability record to update
     */
    void UpdateAbilityRecordLaunchReason(const AbilityRequest &abilityRequest,
        UIAbilityRecordPtr &abilityRecord) const;

    /**
     * @brief Erase ability record from internal storage
     * @param abilityRecord The ability record to erase
     */
    void EraseAbilityRecord(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Dispatch state change to ability
     * @param abilityRecord The ability record to update
     * @param state The new state to set
     * @return ERR_OK if successful, error code otherwise
     */
    int DispatchState(const UIAbilityRecordPtr &abilityRecord, int state);

    /**
     * @brief Dispatch terminate command to ability
     * @param abilityRecord The ability record to terminate
     * @return ERR_OK if successful, error code otherwise
     */
    int DispatchTerminate(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Dispatch background command to ability
     * @param abilityRecord The ability record to background
     * @return ERR_OK if successful, error code otherwise
     */
    int DispatchBackground(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Dispatch foreground command to ability
     * @param abilityRecord The ability record to foreground
     * @param success Whether foreground operation succeeded
     * @param state The ability state to set
     * @return ERR_OK if successful, error code otherwise
     */
    int DispatchForeground(const UIAbilityRecordPtr &abilityRecord, bool success,
        AbilityState state = AbilityState::INITIAL);

    /**
     * @brief Complete foreground success operation
     * @param abilityRecord The ability record that completed foreground
     */
    void CompleteForegroundSuccess(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Handle load timeout for ability
     * @param ability The ability that timed out
     */
    void HandleLoadTimeout(const UIAbilityRecordPtr &ability);

    /**
     * @brief Handle foreground failed for ability
     * @param ability The ability that failed foreground
     * @param state The ability state to set
     */
    void HandleForegroundFailed(const UIAbilityRecordPtr &ability,
        AbilityState state = AbilityState::INITIAL);
    
     /**
     * @brief Handle background for prelaunch
     * @param abilityRecordabilityRecord The ability record that prelaunch
     */
    void HandlePrelaunchBackground(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Handle foreground timeout for ability
     * @param ability The ability that timed out
     */
    void HandleForegroundTimeout(const UIAbilityRecordPtr &ability);

    /**
     * @brief Notify SCB to handle ability exception
     * @param ability The ability with exception
     * @param errorCode The error code
     * @param errorReason The error reason
     * @param needClearCallerLink Whether to clear caller link
     */
    void NotifySCBToHandleException(const UIAbilityRecordPtr &ability, int32_t errorCode,
        const std::string& errorReason, bool needClearCallerLink = true);

    /**
     * @brief Move ability to background state
     * @param abilityRecord The ability record to move
     */
    void MoveToBackground(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Complete background operation for ability
     * @param abilityRecord The ability that completed background
     */
    void CompleteBackground(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Print timeout log message
     * @param ability The ability that timed out
     * @param msgId The message ID
     * @param isHalf Whether it's a half timeout
     */
    void PrintTimeOutLog(UIAbilityRecordPtr ability, uint32_t msgId, bool isHalf = false);

    /**
     * @brief Delay complete terminate operation
     * @param abilityRecord The ability record to terminate
     */
    void DelayCompleteTerminate(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Complete terminate operation
     * @param abilityRecord The ability record to terminate
     */
    void CompleteTerminate(const UIAbilityRecordPtr &abilityRecord);

    /**
     * @brief Check if token is contained in internal storage
     * @param token The token to check
     * @return true if contained, false otherwise
     */
    bool IsContainsAbilityInner(const sptr<IRemoteObject> &token) const;

    /**
     * @brief Check ability properties against request
     * @param abilityRecord The ability record to check
     * @param abilityRequest The ability request to compare
     * @param launchMode The launch mode to verify
     * @return true if properties match, false otherwise
     */
    bool CheckProperties(const UIAbilityRecordPtr &abilityRecord, const AbilityRequest &abilityRequest,
        AppExecFwk::LaunchMode launchMode) const;

    /**
     * @brief Notify ability token change
     * @param token The new token
     * @param abilityRequest The ability request containing change
     */
    void NotifyAbilityToken(const sptr<IRemoteObject> &token, const AbilityRequest &abilityRequest) const;

    /**
     * @brief Prepare to close UI ability
     * @param abilityRecord The ability to close
     * @param resultCode The result code
     * @param resultWant The result want
     * @param isClearSession Whether to clear session
     */
    void PrepareCloseUIAbility(UIAbilityRecordPtr abilityRecord,
        int resultCode, const Want *resultWant, bool isClearSession);

    /**
     * @brief Close UI ability internal implementation
     * @param abilityRecord The ability to close
     * @return ERR_OK if successful, error code otherwise
     */
    int CloseUIAbilityInner(UIAbilityRecordPtr abilityRecord);

    /**
     * @brief Return to caller ability with result (locked)
     * @param currentSessionInfo Current session info
     * @param callerAbilityRecord The caller ability record
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t BackToCallerAbilityWithResultLocked(sptr<SessionInfo> currentSessionInfo,
        std::shared_ptr<AbilityRecord> callerAbilityRecord);

    // byCall
    /**
     * @brief Call ability with lock held
     * @param abilityRequest The ability request
     * @param errMsg Output parameter for error message
     * @return ERR_OK if successful, error code otherwise
     */
    int CallAbilityLocked(const AbilityRequest &abilityRequest, std::string &errMsg);

    /**
     * @brief Create session info for ability
     * @param abilityRequest The ability request
     * @param requestId The request ID
     * @return Newly created session info
     */
    sptr<SessionInfo> CreateSessionInfo(const AbilityRequest &abilityRequest, int32_t requestId);

    /**
     * @brief Notify SCB about pending activation
     * @param sessionInfo The session info
     * @param abilityRequest The ability request
     * @param errMsg Output parameter for error message
     * @return ERR_OK if successful, error code otherwise
     */
    int NotifySCBPendingActivation(sptr<SessionInfo> &sessionInfo,
        const AbilityRequest &abilityRequest, std::string &errMsg);
    void CreateSessionConfigurations(std::vector<sptr<SessionInfo>> &sessionInfoList, int primaryWindowId,
        std::vector<Rosen::PendingSessionActivationConfig> &configList, sptr<SessionInfo> sessionInfo);

    /**
     * @brief Notify SCB about pending activation in split mode
     * @param sessionInfo The session info
     * @param abilityRequest The ability request
     * @return ERR_OK if successful, error code otherwise
     */
    int NotifySCBPendingActivationInSplitMode(sptr<SessionInfo> &sessionInfo, const AbilityRequest &abilityRequest);

    /**
     * @brief Batch notify SCB about pending activations
     * @param abilitiesRequest The abilities request
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t BatchNotifySCBPendingActivations(const AbilitiesRequest &abilitiesRequest);

    /**
     * @brief Handle abilities request completion
     * @param requestId The request ID
     * @param requestListId The request list ID
     * @param sessionInfo The session info
     */
    void HandleAbilitiesRequestDone(int32_t requestId, int32_t requestListId, sptr<SessionInfo> sessionInfo);

    /**
     * @brief Check if ability is hook module
     * @param abilityRequest The ability request
     * @return true if hook module, false otherwise
     */
    bool IsHookModule(const AbilityRequest &abilityRequest) const;

    /**
     * @brief Resolve ability request
     * @param targetAbility The target ability
     * @param abilityRequest The ability request
     * @return ERR_OK if successful, error code otherwise
     */
    int ResolveAbility(UIAbilityRecordPtr targetAbility, const AbilityRequest &abilityRequest) const;

    /**
     * @brief Get ability records by name (internal)
     * @param element The element name
     * @return Vector of matching ability records
     */
    std::vector<UIAbilityRecordPtr> GetAbilityRecordsByNameInner(const AppExecFwk::ElementName &element,
        int32_t appIndex);

    /**
     * @brief Handle foreground collaboration
     * @param abilityRequest The ability request
     * @param abilityRecord The ability record
     */
    void HandleForegroundCollaborate(const AbilityRequest &abilityRequest,
        UIAbilityRecordPtr abilityRecord);

    /**
     * @brief Move ability to front
     * @param specifiedRequest The specified request
     * @param abilityRecord The ability record
     * @param callerAbility The caller ability
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t MoveAbilityToFront(const SpecifiedRequest &specifiedRequest,
        const UIAbilityRecordPtr abilityRecord, UIAbilityRecordPtr callerAbility);

    /**
     * @brief Send session info to SCB in split mode
     * @param primaryWindowId The primary window ID
     * @param callerAbility The caller ability
     * @param sessionInfo The session info
     * @return ERR_OK if successful, error code otherwise
     */
    int SendSessionInfoToSCBInSplitMode(int primaryWindowId, UIAbilityRecordPtr callerAbility,
        sptr<SessionInfo> sessionInfo);

    /**
     * @brief Send session info to SCB
     * @param callerAbility The caller ability
     * @param sessionInfo The session info
     * @return ERR_OK if successful, error code otherwise
     */
    int SendSessionInfoToSCB(UIAbilityRecordPtr &callerAbility, sptr<SessionInfo> &sessionInfo);

    /**
     * @brief Start ability by specified request
     * @param specifiedRequest The specified request
     * @param callerAbility The caller ability
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t StartAbilityBySpecified(const SpecifiedRequest &specifiedRequest,
        UIAbilityRecordPtr callerAbility);
    
    int32_t StartSpecifiedAbilityDirectlyWithFlag(const AbilityRequest &abilityRequest, int32_t requestId);

    /**
     * @brief Set last exit reason for ability
     * @param abilityRecord The ability record
     */
    static void SetLastExitReason(UIAbilityRecordPtr abilityRecord);

    void SetLastExitReasonAsync(UIAbilityRecordPtr abilityRecord);

    void SyncLoadExitReasonTask(int32_t abilityRecordId);

    /**
     * @brief Set receiver info for ability
     * @param abilityRequest The ability request
     * @param abilityRecord The ability record
     */
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

    /**
     * @brief Get content and type ID for message
     * @param msgId The message ID
     * @param msgContent Output parameter for message content
     * @param typeId Output parameter for type ID
     * @return true if successful, false otherwise
     */
    bool GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const;

    /**
     * @brief Send ability event
     * @param abilityInfo The ability info
     * @param reason The event reason
     */
    void SendAbilityEvent(const AppExecFwk::AbilityInfo &abilityInfo, const std::string &reason) const;

    /**
     * @brief Check session info validity
     * @param sessionInfo The session info to check
     * @return true if valid, false otherwise
     */
    bool CheckSessionInfo(sptr<SessionInfo> sessionInfo) const;

    /**
     * @brief Create ability record
     * @param abilityRequest The ability request
     * @param sessionInfo The session info
     * @return Newly created ability record
     */
    UIAbilityRecordPtr CreateAbilityRecord(AbilityRequest &abilityRequest,
        sptr<SessionInfo> sessionInfo) const;

    /**
     * @brief Add caller record to ability
     * @param abilityRequest The ability request
     * @param sessionInfo The session info
     * @param uiAbilityRecord The UI ability record
     */
    void AddCallerRecord(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        UIAbilityRecordPtr uiAbilityRecord) const;

    /**
     * @brief Send key event for ability
     * @param abilityRequest The ability request
     */
    void SendKeyEvent(const AbilityRequest &abilityRequest) const;

    /**
     * @brief Check PID validity
     * @param abilityRecord The ability record
     * @param pid The PID to check
     * @return true if valid, false otherwise
     */
    bool CheckPid(const UIAbilityRecordPtr abilityRecord, const int32_t pid) const;

    /**
     * @brief Get status bar delegate manager
     * @return Shared pointer to status bar delegate manager
     */
    std::shared_ptr<StatusBarDelegateManager> GetStatusBarDelegateManager();

    /**
     * @brief Process ability attachment
     * @param abilityRecord The ability record
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t DoProcessAttachment(UIAbilityRecordPtr abilityRecord);

    /**
     * @brief Batch close UI abilities
     * @param abilitySet The set of abilities to close
     */
    void BatchCloseUIAbility(const std::unordered_set<UIAbilityRecordPtr>& abilitySet);

    /**
     * @brief Terminate session for ability
     * @param abilityRecord The ability record
     */
    void TerminateSession(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * @brief Start ability with persistent ID (distributed)
     * @param abilityRequest The ability request
     * @param persistentId The persistent ID
     * @return ERR_OK if successful, error code otherwise
     */
    int StartWithPersistentIdByDistributed(const AbilityRequest &abilityRequest, int32_t persistentId);

    /**
     * @brief Check if caller is from background
     * @param callerAbility The caller ability
     * @param sessionInfo The session info
     */
    void CheckCallerFromBackground(UIAbilityRecordPtr callerAbility, sptr<SessionInfo> &sessionInfo);

    /**
     * @brief Process caller attachment
     * @param abilityRecord The ability record
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t DoCallerProcessAttachment(UIAbilityRecordPtr abilityRecord);

    /**
     * @brief Process caller detachment
     * @param abilityRecord The ability record
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t DoCallerProcessDetachment(UIAbilityRecordPtr abilityRecord);

    /**
     * @brief Generate ability record from request
     * @param abilityRequest The ability request
     * @param sessionInfo The session info
     * @param isColdStart Output parameter indicating cold start
     * @return Newly created ability record
     */
    UIAbilityRecordPtr GenerateAbilityRecord(AbilityRequest &abilityRequest,
        sptr<SessionInfo> sessionInfo, bool &isColdStart);
    UIAbilityRecordPtr HandleAbilityRecordReused(UIAbilityRecordPtr uiAbilityRecord,
        SessionInfo &sessionInfo, AbilityRequest &abilityRequest);

    /**
     * @brief Find ability record from temporary map
     * @param abilityRequest The ability request
     * @return Found ability record or nullptr
     */
    UIAbilityRecordPtr FindRecordFromTmpMap(const AbilityRequest &abilityRequest);

    /**
     * @brief Post call timeout task
     * @param requestId The request ID to timeout
     */
    void PostCallTimeoutTask(int32_t requestId);

    /**
     * @brief Add caller timestamp
     * @param callerUid The caller UID
     * @return true if added, false if rate limited
     */
    bool AddStartCallerTimestamp(int32_t callerUid);

    /**
     * @brief Find ability record from session map
     * @param abilityRequest The ability request
     * @return Found ability record or nullptr
     */
    UIAbilityRecordPtr FindRecordFromSessionMap(const AbilityRequest &abilityRequest);

    /**
     * @brief Add specified request to queue
     * @param request The specified request
     */
    void AddSpecifiedRequest(std::shared_ptr<SpecifiedRequest> request);

    /**
     * @brief Start processing specified request
     * @param specifiedRequest The request to process
     */
    void StartSpecifiedRequest(SpecifiedRequest &specifiedRequest);

    /**
     * @brief Pop next specified request by ID
     * @param requestId The request ID
     * @return The specified request or nullptr
     */
    std::shared_ptr<SpecifiedRequest> PopAndGetNextSpecified(int32_t requestId);

    /**
     * @brief Check if specified module is loaded
     * @param abilityRequest The ability request
     * @param isSpecifiedProcess Whether it's specified process
     * @param isDebug Output parameter for debug state
     * @return true if module loaded, false otherwise
     */
    bool IsSpecifiedModuleLoaded(const AbilityRequest &abilityRequest, bool isSpecifiedProcess, bool &isDebug);

    /**
     * @brief Handle cold start for specified ability
     * @param abilityRequest The ability request
     * @param sessionInfo The session info
     * @param params start parameters.
     * @return true if handled, false otherwise
     */
    bool HandleStartSpecifiedCold(const AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        AbilityRuntime::StartParamsBySCB &params);

    /**
     * @brief Handle cold accept want completion
     * @param want The accepted want
     * @param flag The completion flag
     * @param specifiedRequest The specified request
     * @return true if handled, false otherwise
     */
    bool HandleColdAcceptWantDone(const AAFwk::Want &want, const std::string &flag,
        const SpecifiedRequest &specifiedRequest);

    /**
     * @brief Handle legacy accept want completion
     * @param specifiedRequest The specified request
     * @param flag The completion flag
     * @param want The accepted want
     */
    void HandleLegacyAcceptWantDone(SpecifiedRequest &specifiedRequest,
        const std::string &flag, const AAFwk::Want &want);

    /**
     * @brief Get specified request by ID
     * @param requestId The request ID
     * @return The specified request or nullptr
     */
    std::shared_ptr<SpecifiedRequest> GetSpecifiedRequest(int32_t requestId);

    /**
     * @brief Check prepare terminate tokens
     * @param tokens The tokens to check
     * @param tokenId Output parameter for token ID
     * @param tokensPerModuleName Output parameter for grouped tokens
     * @return true if valid, false otherwise
     */
    bool CheckPrepareTerminateTokens(const std::vector<sptr<IRemoteObject>> &tokens,
        uint32_t &tokenId, std::map<std::string, std::vector<sptr<IRemoteObject>>> &tokensPerModuleName);

    /**
     * @brief Handle ability stage prepare termination timeout
     * @param pid The process ID
     * @param moduleName The module name
     * @param tokens The ability tokens
     */
    void HandleAbilityStageOnPrepareTerminationTimeout(int32_t pid, const std::string &moduleName,
        const std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * @brief Prepare terminate app and get remaining tokens (internal)
     * @param pid The process ID
     * @param moduleName The module name
     * @param tokens The ability tokens
     * @return Vector of remaining tokens
     */
    std::vector<sptr<IRemoteObject>> PrepareTerminateAppAndGetRemainingInner(int32_t pid, const std::string &moduleName,
        const std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * @brief Cancel prepare terminate operation
     * @param abilityRecord The ability record
     */
    void CancelPrepareTerminate(UIAbilityRecordPtr abilityRecord);

    /**
     * @brief Update specified flag for ability
     * @param abilityRequest The ability record
     * @param flag The flag to set
     * @return true if updated, false otherwise
     */
    bool UpdateSpecifiedFlag(UIAbilityRecordPtr abilityRequest, const std::string &flag);
    void SetProcessPrepareExit(int32_t pid, size_t remainingTokensSize, size_t abilityToTerminateSize,
        bool clear) const;

    /**
     * @brief Process cold start branch
     * @param abilityRequest The ability request
     * @param sessionInfo The session info
     * @param uiAbilityRecord The UI ability record
     * @param isColdStart Whether it's cold start
     * @return true if processed, false otherwise
     */
    bool ProcessColdStartBranch(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        UIAbilityRecordPtr uiAbilityRecord, bool isColdStart);

    /**
     * @brief Try process hook module
     * @param specifiedRequest The specified request
     * @param isHookModule Whether it's hook module
     * @return true if processed, false otherwise
     */
    bool TryProcessHookModule(SpecifiedRequest &specifiedRequest, bool isHookModule);

    /**
     * @brief Check if request is for specified process
     * @param abilityRequest The ability request
     * @return true if specified process request, false otherwise
     */
    bool IsStartSpecifiedProcessRequest(const AbilityRequest &abilityRequest);

    /**
     * @brief Start specified process request
     * @param abilityRequest The ability request
     * @param abilitiesRequest The abilities request
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t StartSpecifiedProcessRequest(const AbilityRequest &abilityRequest,
        std::shared_ptr<AbilitiesRequest> abilitiesRequest);

    /**
     * @brief Handle UI ability request list
     * @param abilityRequestList The ability request list
     * @return ERR_OK if successful, error code otherwise
     */
    int32_t HandleUIAbilityRequestList(std::vector<AbilityRequest> &abilityRequestList);

    /**
     * @brief Handle abilities normal session info
     * @param abilityRequest The ability request
     * @param abilitiesRequest The abilities request
     * @param requestId The request ID
     */
    void HandleAbilitiesNormalSessionInfo(AbilityRequest &abilityRequest,
        std::shared_ptr<AbilitiesRequest> abilitiesRequest, int32_t requestId);

    /**
     * @brief Remove instance key from ability request
     * @param abilityRequest The ability request
     */
    void RemoveInstanceKey(const AbilityRequest &abilityRequest) const;
    bool HandleRestartUIAbility(sptr<SessionInfo> sessionInfo);
    void GetActiveAbilityListLocked(int32_t uid, int32_t pid, std::vector<std::string> &abilityList);
    UIAbilityRecordPtr FindUIAbilityRecordByIdLocked(int64_t abilityRecordId);

    int32_t userId_ = -1;
    mutable ffrt::mutex sessionLock_;
    std::unordered_map<int32_t, UIAbilityRecordPtr> sessionAbilityMap_;
    std::unordered_map<int32_t, UIAbilityRecordPtr> lowMemKillAbilityMap_;
    std::unordered_map<int32_t, UIAbilityRecordPtr> tmpAbilityMap_;
    std::unordered_map<UIAbilityRecordPtr, std::list<AbilityRequest>> callRequestCache_;
    std::list<UIAbilityRecordPtr> terminateAbilityList_;
    // if window reused, persistentId will be reused, keep previous record here until the ability is dead
    std::unordered_set<UIAbilityRecordPtr> reuseWindowRecords_;
    sptr<IRemoteObject> rootSceneSession_;
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

    std::map<int32_t, std::shared_ptr<AbilitiesRequest>> abilitiesRequestMap_;
    std::mutex startingPidsMutex_;
    std::vector<pid_t> startingPids_;

    std::mutex exitReasonTaskMutex_;
    std::unordered_map<int32_t, ffrt::task_handle> exitReasonTasks_; // for sync querying exit-reason task
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H