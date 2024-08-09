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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H

#include <list>
#include <mutex>
#include <queue>
#include <memory>
#include "cpp/mutex.h"

#include "ability_running_info.h"
#include "mission_list.h"
#include "mission_list_manager_interface.h"
#include "mission_listener_controller.h"
#include "mission_info.h"
#include "mission_snapshot.h"
#include "snapshot.h"
#include "start_options.h"
#include "want.h"
#include "iability_info_callback.h"

namespace OHOS {
namespace AAFwk {
class MissionListManager : public MissionListManagerInterface,
                           public std::enable_shared_from_this<MissionListManager> {
public:
    explicit MissionListManager(int userId);
    virtual ~MissionListManager();

    /**
     * init ability mission manager.
     *
     */
    void Init() override;

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbility(AbilityRequest &abilityRequest) override;

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, ability token.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser) override;

    int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    int GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos) override;

    int GetMissionInfo(int32_t missionId, MissionInfo &missionInfo) override;

    int MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions = nullptr) override;

    int MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr) override;

    void NotifyMissionFocused(int32_t missionId) override;

    void NotifyMissionUnfocused(int32_t missionId) override;

    /**
     * OnAbilityRequestDone, app manager service call this interface after ability request done.
     *
     * @param token,ability's token.
     * @param state,the state of ability lift cycle.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) override;

    void OnAppStateChanged(const AppInfo &info) override;

    /**
     * attach ability thread ipc object.
     *
     * @param scheduler, ability thread ipc object.
     * @param token, the token of ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThread(const sptr<AAFwk::IAbilityScheduler> &scheduler,
        const sptr<IRemoteObject> &token) override;

    /**
     * start waiting ability.
     */
    void StartWaitingAbility();

    /**
     * @brief Get the Ability Record By Token object
     *
     * @param token the search token
     * @return std::shared_ptr<AbilityRecord> the AbilityRecord of the token
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token) override;

    /**
     * @brief Get the Mission By Id object
     *
     * @param missionId the given missionId
     * @return the mission of the given id
     */
    std::shared_ptr<Mission> GetMissionById(int missionId) const;

    /**
     * @brief Move ability to background with the given abilityRecord
     *
     * @param abilityRecord the ability to move
     * @return int error code
     */
    int MoveAbilityToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord) override;

    /**
     * @brief Terminate ability with the given abilityRecord
     *
     * @param abilityRecord the ability to terminate
     * @param resultCode the terminate data
     * @param resultWant the terminate data
     * @param flag mark terminate flag
     * @return int error code
     */
    int TerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant, bool flag) override;

    /**
     * @brief remove the mission list from the mission list manager
     *
     * @param MissionList the mission list need to remove
     */
    void RemoveMissionList(const std::shared_ptr<MissionList> &MissionList);

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param state the ability state
     * @param saveData the saved data
     * @return execute error code
     */
    int AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) override;

    /**
     * @brief search the ability from terminating list
     *
     * @param token the ability token
     * @return the ability need to terminate
     */
    std::shared_ptr<AbilityRecord> GetAbilityFromTerminateList(const sptr<IRemoteObject> &token) override;

    /**
     * @brief clear the mission with the given id
     *
     * @param missionId the mission need to delete
     * @return int error code
     */
    int ClearMission(int missionId) override;

    /**
     * @brief clear all the missions
     *
     * @return int error code
     */
    int ClearAllMissions() override;

    void ClearAllMissionsLocked(std::list<std::shared_ptr<Mission>> &missionList,
        std::list<std::shared_ptr<Mission>> &foregroundAbilities, bool searchActive);

    /**
     * @brief Set the Mission Locked State object
     *
     * @param missionId the id of the mission
     * @return int error code
     */
    int SetMissionLockedState(int missionId, bool lockedState) override;

    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    void MoveToBackgroundTask(const std::shared_ptr<AbilityRecord> &abilityRecord, bool isClose = false);

    /**
     * @brief handle time out event
     *
     * @param msgId the msg id in ability record
     * @param abilityRecordId the id of ability record
     * @param isHalf is half
     */
    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false) override;

    /**
     * @brief handle when ability died
     *
     * @param abilityRecord the died ability
     */
    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId) override;

    /**
     * @brief handle when call connection died
     *
     * @param callRecord the died call connection
     */
    void OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord) override;

     /**
     * Get mission id by target ability token.
     *
     * @param token target ability token.
     * @return the missionId of target mission.
     */
    int32_t GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token) override;

    /**
     * Get ability token by target mission id.
     *
     * @param missionId target missionId.
     * @return the ability token of target mission.
     */
    sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId) override;
    std::shared_ptr<AbilityRecord> GetAbilityRecordByMissionId(int32_t missionId) override;

    /**
     * @brief dump all abilities
     *
     * @param info dump result.
     */
    void Dump(std::vector<std::string>& info) override;

    /**
     * @brief dump mission list
     *
     * @param info dump result.
     */
    void DumpMissionList(std::vector<std::string> &info, bool isClient, const std::string &args = "") override;

    /**
     * @brief dump mission list by id with params
     *
     * @param info dump result.
     * @param params dump params.
     */
    void DumpMissionListByRecordId(std::vector<std::string>& info, bool isClient, int32_t abilityRecordId,
        const std::vector<std::string>& params) override;

    /**
     * @brief dump mission by id
     *
     * @param info dump result.
     */
    void DumpMission(int missionId, std::vector<std::string> &info) override;

    /**
     * @brief dump mission infos
     *
     * @param info dump result.
     */
    void DumpMissionInfos(std::vector<std::string> &info) override;

    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag) override;

    void OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want) override;
    /**
     * resolve the call ipc of ability for scheduling oncall.
     *
     * @param abilityRequest, target ability request.
     */
    int ResolveLocked(const AbilityRequest &abilityRequest) override;

    /**
     * release the connection of this call.
     *
     * @param connect, caller callback ipc.
     * @param element, target ability name.
     */
    int ReleaseCallLocked(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element) override;
    /**
     * @brief register snapshotHandler
     * @param handler the snapshotHandler
     */
    void RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;

    /**
     * @brief Get the Mission Snapshot object
     * @param missionId mission id
     * @param abilityToken abilityToken to get current mission snapshot
     * @param missionSnapshot result of snapshot
     * @param isLowResolution low resolution.
     * @return Returns true on success, false on failure.
     */
    bool GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
        MissionSnapshot& missionSnapshot, bool isLowResolution) override;
    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) override;

    /**
     * Called to update mission snapshot.
     * @param token The target ability.
     * @param pixelMap The snapshot.
     */
#ifdef SUPPORT_SCREEN
    void UpdateSnapShot(const sptr<IRemoteObject> &token, const std::shared_ptr<Media::PixelMap> &pixelMap) override;
#endif // SUPPORT_SCREEN

    /**
     * Get ability number.
     * @param element type of ElementName.
     * @return ability number.
     */
    int32_t GetAbilityNumber(const AppExecFwk::ElementName &element) const;

    void EnableRecoverAbility(int32_t missionId) override;

#ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability.
     *
     * @param abilityRecordId The Ability Record Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    int BlockAbility(int abilityRecordId) override;
#endif

    void UninstallApp(const std::string &bundleName, int32_t uid) override;

    bool IsStarted() override;
    void PauseManager() override;
    void ResumeManager() override;

    void SetMissionANRStateByTokens(const std::vector<sptr<IRemoteObject>> &tokens);

    int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds,
        std::vector<MissionValidResult> &results) override;

    int DoAbilityForeground(std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t flag) override;

    void GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityList, int32_t pid = NO_PID) override;

    void CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<IRemoteObject> &callStub) override;

    int SetMissionContinueState(const sptr<IRemoteObject> &token, int32_t missionId,
        const AAFwk::ContinueState &state) override;

    bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord) override;

    void SignRestartAppFlag(const std::string &bundleName) override;

    void SetAnimationFlag(bool IsAnimationEnabled);
#ifdef SUPPORT_SCREEN
public:
    /**
     * Set mission label of this ability.
     *
     * @param abilityToken target ability token.
     * @param label target label.
     * @return Return 0 if success.
     */
    int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) override;

    /**
     * Set mission icon of this ability.
     *
     * @param token target ability token.
     * @param icon target label.
     * @return Return 0 if success.
     */
    int SetMissionIcon(const sptr<IRemoteObject> &token, const std::shared_ptr<Media::PixelMap> &icon) override;

    void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override;

    void PostMissionLabelUpdateTask(int missionId) const;

    void UpdateAbilityRecordColdStartFlag(const AppInfo& info, bool isColdStart);

private:
    Closure GetCancelStartingWindowTask(const std::shared_ptr<AbilityRecord> &abilityRecord) const;
    void PostCancelStartingWindowTask(const std::shared_ptr<AbilityRecord> &abilityRecord) const;
    void InitPrepareTerminateConfig();
#endif

private:
    void AddRecord(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetAbilityRecord);
    int GetTargetMission(const AbilityRequest &abilityRequest, std::shared_ptr<Mission> &targetMission,
        std::shared_ptr<AbilityRecord> &targetAbilityRecord);
    int StartAbilityLocked(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    int StartAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    int MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser);
    std::shared_ptr<AbilityRecord> GetCurrentTopAbilityLocked() const;
    std::shared_ptr<MissionList> GetTargetMissionList(
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    std::shared_ptr<MissionList> GetTargetMissionListByLauncher(const AbilityRequest &abilityRequest);
    std::shared_ptr<MissionList> GetTargetMissionListByDefault(
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    std::shared_ptr<Mission> GetReusedMission(const AbilityRequest &abilityRequest);
    std::shared_ptr<Mission> GetReusedSpecifiedMission(const AbilityRequest &abilityRequest);
    std::shared_ptr<Mission> GetReusedStandardMission(const AbilityRequest &abilityRequest);
    void GetTargetMissionAndAbility(const AbilityRequest &abilityRequest, std::shared_ptr<Mission> &targetMission,
        std::shared_ptr<AbilityRecord> &targetRecord, bool &isReachToLimit);
    bool HandleReusedMissionAndAbility(const AbilityRequest &abilityRequest, std::shared_ptr<Mission> &targetMission,
        std::shared_ptr<AbilityRecord> &targetRecord);
    std::string GetMissionName(const AbilityRequest &abilityRequest) const;
    bool CreateOrReusedMissionInfo(const AbilityRequest &abilityRequest, InnerMissionInfo &info) const;
    void MoveMissionToTargetList(bool isCallFromLauncher,
        const std::shared_ptr<MissionList> &targetMissionList,
        const std::shared_ptr<Mission> &mission);
    void MoveMissionListToTop(const std::shared_ptr<MissionList> &missionList);
    void MoveNoneTopMissionToDefaultList(const std::shared_ptr<Mission> &mission);
    void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId, bool isHalf = false);

    int DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, bool success,
        AbilityState state = AbilityState::INITIAL);
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteForegroundSuccess(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void DelayCompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminateAndUpdateMission(const std::shared_ptr<AbilityRecord> &abilityRecord);
    bool RemoveMissionList(const std::list<std::shared_ptr<MissionList>> lists,
        const std::shared_ptr<MissionList> &list);
    int ClearMissionLocked(int missionId, const std::shared_ptr<Mission> &mission);
    int ClearMissionLocking(int missionId, const std::shared_ptr<Mission> &mission);
    int MoveAbilityToBackgroundLocked(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void RemoveBackgroundingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int TerminateAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool flag);
    /**
     * @brief remove the mission from the mission list
     *
     * @param abilityRecord the ability need to remove
     * @param flag mark is terminate or close
     */
    void RemoveTerminatingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool flag);
    std::shared_ptr<AbilityRecord> GetAbilityRecordById(int64_t abilityRecordId) const;
    std::shared_ptr<AbilityRecord> GetAbilityRecordByCaller(
        const std::shared_ptr<AbilityRecord> &caller, int requestCode);
    std::shared_ptr<MissionList> GetTargetMissionList(int missionId, std::shared_ptr<Mission> &mission,
        bool &isReachToLimit);
    void PostStartWaitingAbility();
    void HandleAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);
    void HandleLauncherDied(std::shared_ptr<AbilityRecord> ability);
    void HandleAbilityDiedByDefault(std::shared_ptr<AbilityRecord> abilityRecord);
    void DelayedStartLauncher();
    void BackToLauncher();
    void GetAllForegroundAbilities(std::list<std::shared_ptr<AbilityRecord>>& foregroundList);
    void GetForegroundAbilities(const std::shared_ptr<MissionList>& missionList,
        std::list<std::shared_ptr<AbilityRecord>>& foregroundList);
    std::shared_ptr<Mission> GetMissionBySpecifiedFlag(const AAFwk::Want &want, const std::string &flag) const;
    bool IsReachToSingleLimitLocked(const int32_t uid) const;
    bool IsReachToLimitLocked() const;
    bool CheckSingleLimit(const AbilityRequest &abilityRequest);
    bool CheckLimit();
    std::shared_ptr<Mission> FindEarliestMission() const;
    int32_t GetMissionCount() const;

    // handle timeout event
    void HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void HandleForegroundTimeout(const std::shared_ptr<AbilityRecord> &ability,
        AbilityState state = AbilityState::INITIAL);
    void HandleTimeoutAndResumeAbility(const std::shared_ptr<AbilityRecord> &ability,
        AbilityState state = AbilityState::INITIAL);
    void MoveToTerminateList(const std::shared_ptr<AbilityRecord> &ability);
    void DelayedResumeTimeout(const std::shared_ptr<AbilityRecord> &callerAbility);
    void BackToCaller(const std::shared_ptr<AbilityRecord> &callerAbility);

    // new version for call inner function.
    void CompleteForegroundFailed(const std::shared_ptr<AbilityRecord> &abilityRecord, AbilityState state);
    int ResolveAbility(const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest);
    std::shared_ptr<AbilityRecord> GetAbilityRecordByName(const AppExecFwk::ElementName &element);
    std::shared_ptr<AbilityRecord> GetAbilityRecordByNameFromCurrentMissionLists(
        const AppExecFwk::ElementName &element) const;
    std::vector<std::shared_ptr<AbilityRecord>> GetAbilityRecordsByName(const AppExecFwk::ElementName &element);
    int CallAbilityLocked(const AbilityRequest &abilityRequest);
    void UpdateMissionSnapshot(const std::shared_ptr<AbilityRecord> &abilityRecord) const;
    void AddUninstallTags(const std::string &bundleName, int32_t uid);
    void EraseWaitingAbility(const std::string &bundleName, int32_t uid);
    void RemoveMissionLocked(int32_t missionId, bool excludeFromMissions);
    void TerminatePreviousAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void NotifyMissionCreated(const std::shared_ptr<AbilityRecord> &abilityRecord) const;
    bool IsExcludeFromMissions(const std::shared_ptr<Mission> &mission);
    void BuildInnerMissionInfo(InnerMissionInfo &info, const std::string &missionName,
        const AbilityRequest &abilityRequest) const;
    void NotifyStartSpecifiedAbility(AbilityRequest &request, const AAFwk::Want &want);
    void NotifyRestartSpecifiedAbility(AbilityRequest &request, const sptr<IRemoteObject> &token);
    void ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const;
    bool UpdateAbilityRecordLaunchReason(
        const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetAbilityRecord);
    std::shared_ptr<AbilityRecord> GetAliveAbilityRecordByToken(const sptr<IRemoteObject> &token) const;
    void NotifyAbilityToken(const sptr<IRemoteObject> &token, const AbilityRequest &abilityRequest);
    void NotifyStartAbilityResult(const AbilityRequest &abilityRequest, int result);
    int MoveMissionToFrontInner(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr);
    /**
     * push waiting ability to queue.
     *
     * @param abilityRequest, the request of ability.
     */
    void EnqueueWaitingAbility(const AbilityRequest &abilityRequest);

    /**
     * push front waiting ability to queue.
     *
     * @param abilityRequest, the request of ability.
     */
    void EnqueueWaitingAbilityToFront(const AbilityRequest &abilityRequest);
    std::shared_ptr<AbilityRecord> GetAbilityRecordByTokenInner(const sptr<IRemoteObject> &token) const;
    int TerminateAbilityInner(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant, bool flag);
    int32_t GetMissionIdByAbilityTokenInner(const sptr<IRemoteObject> &token);
    std::shared_ptr<AbilityRecord> GetAbilityFromTerminateListInner(const sptr<IRemoteObject> &token);
    void SetLastExitReason(std::shared_ptr<AbilityRecord> &abilityRecord);
    bool IsAppLastAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    std::shared_ptr<MissionList> GetMissionList(int32_t missionId);
    int PrepareClearMissionLocked(int missionId, const std::shared_ptr<Mission> &mission);

    bool CheckPrepareTerminateEnable(const std::shared_ptr<Mission> &mission);

    bool GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const;

    void SendKeyEvent(const AbilityRequest &abilityRequest);

    int userId_;
    mutable ffrt::mutex managerLock_;
    // launcher list is also in currentMissionLists_
    std::list<std::shared_ptr<MissionList>> currentMissionLists_;
    // only manager the ability of standard in the default list
    std::shared_ptr<MissionList> defaultStandardList_;
    // only manager the ability of singleton in the default list for the fast search
    std::shared_ptr<MissionList> defaultSingleList_;
    std::shared_ptr<MissionList> launcherList_;
    std::list<std::shared_ptr<AbilityRecord>> terminateAbilityList_;

    std::queue<AbilityRequest> waitingAbilityQueue_;
    std::shared_ptr<MissionListenerController> listenerController_;
    bool isPrepareTerminateEnable_ = false;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H
