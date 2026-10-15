/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "ability_running_info.h"
#include "ffrt.h"
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

    /**
     * @brief Registers a mission listener with the mission list manager.
     *
     * @param listener A shared pointer to the IMissionListener interface to be registered.
     * @return An integer indicating the result of the registration operation (e.g., success or error code).
     */
    int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    /**
     * @brief Unregisters a mission listener from the mission list manager.
     *
     * @param listener A shared pointer to the IMissionListener interface to be unregistered.
     * @return An integer indicating the result of the unregistration operation (e.g., success or error code).
     */
    int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    /**
     * @brief Retrieves mission information from the mission list manager.
     *
     * @param numMax The maximum number of mission infos to retrieve.
     * @param missionInfos A reference to a vector that will be filled with the retrieved mission infos.
     * @return An integer indicating the result of the retrieval operation (e.g., number of mission infos retrieved or
     * error code).
     */
    int GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos) override;

    /**
     * @brief Retrieves information for a specific mission.
     *
     * @param missionId The ID of the mission for which to retrieve information.
     * @param missionInfo A reference to a MissionInfo structure that will be filled with the retrieved mission
     * information.
     * @return An integer indicating the result of the retrieval operation (e.g., success or error code).
     */
    int GetMissionInfo(int32_t missionId, MissionInfo &missionInfo) override;

    /**
     * @brief Moves a mission to the front of the mission list.
     *
     * @param missionId The ID of the mission to move.
     * @param startOptions Optional start options for the mission.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions = nullptr) override;

    /**
     * @brief Moves a mission to the front of the mission list with additional context.
     *
     * @param missionId The ID of the mission to move.
     * @param isCallerFromLauncher A boolean indicating whether the caller is from the launcher.
     * @param isRecent A boolean indicating whether the mission is recent.
     * @param callerAbility A shared pointer to the AbilityRecord of the caller.
     * @param startOptions Optional start options for the mission.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr) override;

    /**
     * @brief Notifies the mission list manager that a mission has been focused.
     *
     * @param missionId The ID of the focused mission.
     */
    void NotifyMissionFocused(int32_t missionId) override;

    /**
     * @brief Notifies the mission list manager that a mission has been unfocused.
     *
     * @param missionId The ID of the unfocused mission.
     */
    void NotifyMissionUnfocused(int32_t missionId) override;

    /**
     * OnAbilityRequestDone, app manager service call this interface after ability request done.
     *
     * @param token,ability's token.
     * @param state,the state of ability lift cycle.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) override;

    /**
     * @brief Handles changes in the state of an application.
     *
     * @param info A reference to an AppInfo structure containing details about the application whose state has changed.
     */
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
     * @brief Back to caller ability with result
     *
     * @param abilityRecord the ability to move
     * @param resultCode result code
     * @param resultWant result want
     * @param callerRequestCode request code of caller
     * @return int error code
     */
    int32_t BackToCallerAbilityWithResult(std::shared_ptr<AbilityRecord> abilityRecord,
        int32_t resultCode, const Want *resultWant, int64_t callerRequestCode) override;

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

    /**
     * @brief Clears all missions from the specified lists, optionally searching for active missions.
     *
     * @param missionList A reference to a list of shared pointers to Mission objects, which will be cleared.
     * @param foregroundAbilities A reference to a list of shared pointers to Mission objects representing foreground
     * abilities, which will be cleared.
     * @param searchActive A boolean indicating whether to search for and clear active missions.
     */
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

    void OnStartSpecifiedAbilityTimeoutResponse() override;
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
     * @param missionSnapshot result of snapshot
     * @param isLowResolution low resolution.
     * @return Returns true on success, false on failure.
     */
    bool GetMissionSnapshot(int32_t missionId, MissionSnapshot& missionSnapshot, bool isLowResolution) override;
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

    void UninstallApp(const std::string &bundleName, int32_t uid) override;

    bool IsStarted() override;
    void PauseManager() override;
    void ResumeManager() override;

    /**
     * @brief Sets the state for missions using the provided tokens.
     *
     * @param tokens A vector of remote object tokens representing the missions.
     */
    void SetMissionANRStateByTokens(const std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * @brief Validates a list of mission IDs and returns the validation results.
     *
     * @param missionIds A vector containing the mission IDs to validate.
     * @param results A vector that will be filled with MissionValidResult objects indicating the validity of each
     * mission ID.
     * @return An integer indicating the result of the validation operation (e.g., success or error code).
     */
    int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds,
        std::vector<MissionValidResult> &results) override;

    /**
     * @brief Brings an ability to the foreground.
     *
     * @param abilityRecord A shared pointer to the AbilityRecord of the ability to bring to the foreground.
     * @param flag A uint32_t flag that may contain additional options or instructions for bringing the ability to the
     * foreground.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int DoAbilityForeground(std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t flag) override;

    /**
     * @brief Retrieves the list of active abilities for a specified user.
     *
     * @param uid The user ID for which to retrieve the active ability list.
     * @param abilityList A vector that will be filled with the names of the active abilities.
     * @param pid An optional process ID to filter the active abilities by. Defaults to NO_PID to include all processes.
     */
    void GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityList, int32_t pid = NO_PID) override;

    /**
     * @brief Notifies that a request for an ability has been completed.
     *
     * @param abilityRecord A shared pointer to the AbilityRecord of the ability for which the request has been
     * completed.
     * @param callStub A shared pointer to the IRemoteObject representing the call stub used for the request.
     */
    void CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<IRemoteObject> &callStub) override;

    /**
     * @brief Sets the continue state for a mission.
     *
     * @param token A shared pointer to the IRemoteObject representing a token used for the operation.
     * @param missionId The ID of the mission for which to set the continue state.
     * @param state The ContinueState to set for the mission.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int SetMissionContinueState(const sptr<IRemoteObject> &token, int32_t missionId,
        const AAFwk::ContinueState &state) override;

    /**
     * @brief Checks whether an ability has been started.
     *
     * @param abilityRequest A reference to the AbilityRequest object containing the details of the ability to check.
     * @param targetRecord A shared pointer to the AbilityRecord that will be set to the started ability if found.
     * @return A boolean indicating whether the ability has been started.
     */
    bool IsAbilityStarted(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord) override;

    /**
     * @brief Signs an application for restart based on the provided user ID and instance key.
     *
     * @param uid The user ID of the application to sign for restart.
     * @param instanceKey The instance key of the application to sign for restart.
     */
    void SignRestartAppFlag(int32_t uid, const std::string &instanceKey) override;

    /**
     * @brief Sets the flag indicating whether animations are enabled.
     *
     * @param IsAnimationEnabled A boolean indicating whether animations should be enabled.
     */
    void SetAnimationFlag(bool IsAnimationEnabled);
#ifdef SUPPORT_SCREEN
public:
    /**
     * @brief Set mission label of this ability.
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

    /**
     * @brief Completes the drawing of the first frame for the ability associated with the given remote object token.
     *
     * @param abilityToken The remote object token of the ability for which to complete the first frame drawing.
     */
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override;

    /**
     * @brief Posts a task to update the mission label for the specified mission ID.
     *
     * @param missionId The unique identifier of the mission whose label needs to be updated.
     */
    void PostMissionLabelUpdateTask(int missionId) const;

    /**
     * @brief Updates the cold start flag for the ability record based on the provided app info.
     *
     * @param info         Contains the application-related data that is used to determine the cold start status.
     * @param isColdStart  A boolean value indicating whether the ability is starting cold or not.
     */
    void UpdateAbilityRecordColdStartFlag(const AppInfo& info, bool isColdStart);

private:
    /**
     * @brief Retrieves a closure that can be used to cancel the starting window task
     *        for the specified ability record.
     *
     * @param abilityRecord A shared pointer to the ability record associated with the
     *                      starting window task.
     * @return A closure that, when invoked, will cancel the starting window task.
     */
    Closure GetCancelStartingWindowTask(const std::shared_ptr<AbilityRecord> &abilityRecord) const;

    /**
     * @brief Posts a task to cancel the starting window for the specified ability record.
     *
     * @param abilityRecord A shared pointer to the ability record whose starting window
     *                      should be canceled.
     */
    void PostCancelStartingWindowTask(const std::shared_ptr<AbilityRecord> &abilityRecord) const;

    /**
     * @brief Initializes the configuration for preparing to terminate abilities.
     *
     * This function sets up any necessary state or resources that will be used when
     * terminating abilities, such as cleanup tasks or resource management.
     */
    void InitPrepareTerminateConfig();
#endif

private:
    /**
     * @brief Adds a new ability record to the system based on the provided ability request.
     *
     * @param abilityRequest The request that specifies the ability to be added.
     * @param targetAbilityRecord A reference to a shared pointer that will hold the new ability record.
     */
    void AddRecord(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> targetAbilityRecord);

    /**
     * @brief Retrieves the target mission and ability record based on the ability request.
     *
     * @param abilityRequest The request that specifies the ability to be started.
     * @param targetMission A reference to a shared pointer that will hold the target mission.
     * @param targetAbilityRecord A reference to a shared pointer that will hold the target ability record.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int GetTargetMission(const AbilityRequest &abilityRequest, std::shared_ptr<Mission> &targetMission,
        MissionAbilityRecordPtr &targetAbilityRecord);

    /**
     * @brief Starts an ability in a locked context, ensuring thread safety.
     *
     * @param currentTopAbility The currently top-most ability.
     * @param callerAbility The ability that is initiating the start request.
     * @param abilityRequest The request that specifies the ability to be started.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int StartAbilityLocked(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);

    /**
     * @brief Starts an ability.
     *
     * @param currentTopAbility The currently top-most ability.
     * @param callerAbility The ability that is initiating the start request.
     * @param abilityRequest The request that specifies the ability to be started.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int StartAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);

    /**
     * @brief Minimizes an ability in a locked context, ensuring thread safety.
     *
     * @param abilityRecord The ability record to be minimized.
     * @param fromUser A boolean indicating whether the minimization was initiated by the user.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser);

    /**
     * @brief Retrieves the currently top-most ability record in a locked context.
     *
     * @return A shared pointer to the currently top-most ability record.
     */
    std::shared_ptr<AbilityRecord> GetCurrentTopAbilityLocked() const;

    /**
     * @brief Retrieves the target mission list based on the caller ability and ability request.
     *
     * @param callerAbility The ability that is initiating the request.
     * @param abilityRequest The request that specifies the ability.
     * @return A shared pointer to the target mission list.
     */
    std::shared_ptr<MissionList> GetTargetMissionList(
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);

    /**
     * @brief Retrieves the target mission list based on the ability request, assuming it is from a launcher.
     *
     * @param abilityRequest The request that specifies the ability.
     * @return A shared pointer to the target mission list.
     */
    std::shared_ptr<MissionList> GetTargetMissionListByLauncher(const AbilityRequest &abilityRequest);

    /**
     * @brief Retrieves the target mission list based on default criteria, using the caller ability and ability request.
     *
     * @param callerAbility The ability that is initiating the request.
     * @param abilityRequest The request that specifies the ability.
     * @return A shared pointer to the target mission list.
     */
    std::shared_ptr<MissionList> GetTargetMissionListByDefault(
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);

    /**
     * @brief Retrieves a reused mission based on the ability request.
     *
     * @param abilityRequest The request that specifies the ability.
     * @return A shared pointer to the reused mission.
     */
    std::shared_ptr<Mission> GetReusedMission(const AbilityRequest &abilityRequest);

    /**
     * @brief Retrieves a reused mission with specified criteria based on the ability request.
     *
     * @param abilityRequest The request that specifies the ability.
     * @return A shared pointer to the reused mission.
     */
    std::shared_ptr<Mission> GetReusedSpecifiedMission(const AbilityRequest &abilityRequest);

    /**
     * @brief Retrieves a reused standard mission based on the ability request.
     *
     * @param abilityRequest The request that specifies the ability.
     * @return A shared pointer to the reused standard mission.
     */
    std::shared_ptr<Mission> GetReusedStandardMission(const AbilityRequest &abilityRequest);

    /**
     * @brief Retrieves the target mission and ability record, and checks if the limit is reached.
     *
     * @param abilityRequest The request that specifies the ability.
     * @param targetMission A reference to a shared pointer that will hold the target mission.
     * @param targetRecord A reference to a shared pointer that will hold the target ability record.
     * @param isReachToLimit A reference to a boolean that will indicate whether the limit is reached.
     */
    void GetTargetMissionAndAbility(const AbilityRequest &abilityRequest, std::shared_ptr<Mission> &targetMission,
        MissionAbilityRecordPtr &targetRecord, bool &isReachToLimit);

    /**
     * @brief Handles the reuse of a mission and ability based on the ability request.
     *
     * @param abilityRequest The request that specifies the ability.
     * @param targetMission A reference to a shared pointer that will hold the target mission.
     * @param targetRecord A reference to a shared pointer that will hold the target ability record.
     * @return A boolean indicating whether the reuse was successful.
     */
    bool HandleReusedMissionAndAbility(const AbilityRequest &abilityRequest, std::shared_ptr<Mission> &targetMission,
        MissionAbilityRecordPtr &targetRecord);

    /**
     * @brief Retrieves the name of the mission based on the ability request.
     *
     * @param abilityRequest The request that specifies the ability.
     * @return A string containing the mission name.
     */
    std::string GetMissionName(const AbilityRequest &abilityRequest) const;

    /**
     * @brief Creates or retrieves mission info based on the ability request.
     *
     * @param abilityRequest The request that specifies the ability.
     * @param info A reference to an InnerMissionInfo object that will hold the mission info.
     * @return A boolean indicating whether the operation was successful.
     */
    bool CreateOrReusedMissionInfo(const AbilityRequest &abilityRequest, InnerMissionInfo &info) const;

    /**
     * @brief Moves a mission to the target list.
     *
     * @param isCallFromLauncher A boolean indicating whether the call is from a launcher.
     * @param targetMissionList A shared pointer to the target mission list.
     * @param mission A shared pointer to the mission to be moved.
     */
    void MoveMissionToTargetList(bool isCallFromLauncher,
        const std::shared_ptr<MissionList> &targetMissionList,
        const std::shared_ptr<Mission> &mission);

    /**
     * @brief Moves a mission list to the top of the stack.
     *
     * @param missionList A shared pointer to the mission list to be moved.
     */
    void MoveMissionListToTop(const std::shared_ptr<MissionList> &missionList);

    /**
     * @brief Moves a non-top mission to the default list.
     *
     * @param mission A shared pointer to the mission to be moved.
     */
    void MoveNoneTopMissionToDefaultList(const std::shared_ptr<Mission> &mission);

    /**
     * @brief Prints a timeout log for the specified ability.
     *
     * @param ability A shared pointer to the ability.
     * @param msgId The message ID associated with the timeout.
     * @param isHalf A boolean indicating whether it is a half timeout.
     */
    void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId, bool isHalf = false);

    /**
     * @brief Dispatches a state change event for the specified ability record.
     *
     * @param abilityRecord The ability record for which the state change is being dispatched.
     * @param state The new state to which the ability record is transitioning.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int DispatchState(MissionAbilityRecordPtr abilityRecord, int state);

    /**
     * @brief Dispatches a foreground event for the specified ability record, indicating whether the operation was
     * successful.
     *
     * @param abilityRecord The ability record for which the foreground event is being dispatched.
     * @param success A boolean indicating whether the ability successfully moved to the foreground.
     * @param state The initial state of the ability (default is AbilityState::INITIAL).
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int DispatchForeground(MissionAbilityRecordPtr abilityRecord, bool success,
        AbilityState state = AbilityState::INITIAL);

    /**
     * @brief Dispatches a terminate event for the specified ability record.
     *
     * @param abilityRecord The ability record for which the terminate event is being dispatched.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int DispatchTerminate(MissionAbilityRecordPtr abilityRecord);

    /**
     * @brief Dispatches a background event for the specified ability record.
     *
     * @param abilityRecord The ability record for which the background event is being dispatched.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int DispatchBackground(MissionAbilityRecordPtr abilityRecord);

    /**
     * @brief Completes the foreground transition for the specified ability record, indicating success.
     *
     * @param abilityRecord The ability record for which the foreground transition is being completed.
     */
    void CompleteForegroundSuccess(MissionAbilityRecordPtr abilityRecord);

    /**
     * @brief Completes the termination process for the specified ability record.
     *
     * @param abilityRecord The ability record for which the termination is being completed.
     */
    void CompleteTerminate(MissionAbilityRecordPtr abilityRecord);

    /**
     * @brief Delays the completion of the termination process for the specified ability record.
     *
     * @param abilityRecord The ability record for which the termination completion is being delayed.
     */
    void DelayCompleteTerminate(MissionAbilityRecordPtr abilityRecord);

    /**
     * @brief Completes the background transition for the specified ability record.
     *
     * @param abilityRecord The ability record for which the background transition is being completed.
     */
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * @brief Completes the termination process for the specified ability record and updates the associated mission.
     *
     * @param abilityRecord The ability record for which the termination and mission update are being performed.
     */
    void CompleteTerminateAndUpdateMission(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * @brief Removes a mission list from the specified list of mission lists.
     *
     * @param lists A list of shared pointers to mission lists.
     * @param list The mission list to be removed.
     * @return A boolean indicating whether the mission list was successfully removed.
     */
    bool RemoveMissionList(const std::list<std::shared_ptr<MissionList>> lists,
        const std::shared_ptr<MissionList> &list);

    /**
     * @brief Clears a mission in a locked context, ensuring thread safety.
     *
     * @param missionId The unique identifier of the mission to be cleared.
     * @param mission A shared pointer to the mission to be cleared.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int ClearMissionLocked(int missionId, const std::shared_ptr<Mission> &mission);

    /**
     * @brief Clears a mission while acquiring a lock, ensuring thread safety.
     *
     * @param missionId The unique identifier of the mission to be cleared.
     * @param mission A shared pointer to the mission to be cleared.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int ClearMissionLocking(int missionId, const std::shared_ptr<Mission> &mission);

    /**
     * @brief Moves the specified ability record to the background in a locked context, optionally specifying the next
     * record.
     *
     * @param abilityRecord The ability record to move to the background.
     * @param specifiedNextRecord An optional ability record to set as the next record after the move.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int MoveAbilityToBackgroundLocked(MissionAbilityRecordPtr abilityRecord,
        MissionAbilityRecordPtr specifiedNextRecord = nullptr);

    /**
     * @brief Removes an ability record from the backgrounding process.
     *
     * @param abilityRecord The ability record to remove from the backgrounding process.
     */
    void RemoveBackgroundingAbility(MissionAbilityRecordPtr abilityRecord);

    /**
     * @brief Terminates an ability record in a locked context, ensuring thread safety.
     *
     * @param abilityRecord The ability record to be terminated.
     * @param flag A boolean flag that may influence the termination process.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int TerminateAbilityLocked(MissionAbilityRecordPtr abilityRecord, bool flag);

    /**
     * @brief remove the mission from the mission list
     *
     * @param abilityRecord the ability need to remove
     * @param flag mark is terminate or close
     */
    void RemoveTerminatingAbility(MissionAbilityRecordPtr abilityRecord, bool flag);

    /**
     * @brief Retrieves an ability record by its unique identifier.
     *
     * @param abilityRecordId The unique identifier of the ability record to retrieve.
     * @return A shared pointer to the ability record, or a null pointer if not found.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordById(int64_t abilityRecordId) const;

    /**
     * @brief Retrieves an ability record by the caller ability and request code.
     *
     * @param caller The ability record that initiated the request.
     * @param requestCode The request code associated with the ability to retrieve.
     * @return A shared pointer to the ability record, or a null pointer if not found.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByCaller(
        const std::shared_ptr<AbilityRecord> &caller, int requestCode);

    /**
     * @brief Retrieves the target mission list for a given mission ID.
     *
     * @param missionId The unique identifier of the mission.
     * @param mission A reference to a shared pointer that will hold the retrieved mission.
     * @param isReachToLimit A reference to a boolean that will indicate whether the limit has been reached.
     * @return A shared pointer to the target mission list, or a null pointer if not found.
     */
    std::shared_ptr<MissionList> GetTargetMissionList(int missionId, std::shared_ptr<Mission> &mission,
        bool &isReachToLimit);

    /**
     * @brief Posts a start waiting ability event.
     */
    void PostStartWaitingAbility();

    /**
     * @brief Handles the death of an ability record.
     *
     * @param abilityRecord The ability record that has died.
     */
    void HandleAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * @brief Handles the death of a launcher ability.
     *
     * @param ability The launcher ability that has died.
     */
    void HandleLauncherDied(std::shared_ptr<AbilityRecord> ability);

    /**
     * @brief Handles the death of an ability record using default handling logic.
     *
     * @param abilityRecord The ability record that has died.
     */
    void HandleAbilityDiedByDefault(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * @brief Delayed start of a launcher ability.
     */
    void DelayedStartLauncher();

    /**
     * @brief Navigates back to the launcher.
     */
    void BackToLauncher();

    /**
     * @brief Retrieves all foreground ability records.
     *
     * @param foregroundList A reference to a list that will hold the retrieved foreground ability records.
     */
    void GetAllForegroundAbilities(std::list<std::shared_ptr<AbilityRecord>>& foregroundList);

    /**
     * @brief Retrieves foreground ability records for a given mission list.
     *
     * @param missionList The mission list for which to retrieve foreground abilities.
     * @param foregroundList A reference to a list that will hold the retrieved foreground ability records.
     */
    void GetForegroundAbilities(const std::shared_ptr<MissionList>& missionList,
        std::list<std::shared_ptr<AbilityRecord>>& foregroundList);

    /**
     * @brief Retrieves a mission by a specified flag in the want.
     *
     * @param want The want object containing the flag.
     * @param flag The specific flag to match.
     * @return A shared pointer to the mission, or a null pointer if not found.
     */
    std::shared_ptr<Mission> GetMissionBySpecifiedFlag(const AAFwk::Want &want, const std::string &flag) const;

    /**
     * @brief Checks if the single limit has been reached for a given UID in a locked context.
     *
     * @param uid The unique identifier of the user.
     * @return A boolean indicating whether the single limit has been reached.
     */
    bool IsReachToSingleLimitLocked(const int32_t uid) const;

    /**
     * @brief Checks if the general limit has been reached in a locked context.
     *
     * @return A boolean indicating whether the limit has been reached.
     */
    bool IsReachToLimitLocked() const;

    /**
     * @brief Checks if a specific ability request exceeds the single limit.
     *
     * @param abilityRequest The ability request to check.
     * @return A boolean indicating whether the single limit has been exceeded.
     */
    bool CheckSingleLimit(const AbilityRequest &abilityRequest);

    /**
     * @brief Checks if the general limit has been reached.
     *
     * @return A boolean indicating whether the limit has been reached.
     */
    bool CheckLimit();

    /**
     * @brief Finds the earliest mission in the system.
     *
     * @return A shared pointer to the earliest mission, or a null pointer if no missions exist.
     */
    std::shared_ptr<Mission> FindEarliestMission() const;

    /**
     * @brief Retrieves the total count of missions in the system.
     *
     * @return An integer representing the total count of missions.
     */
    int32_t GetMissionCount() const;

    // handle timeout event
    /**
     * @brief Handles a load timeout event for a given ability record.
     *
     * @param ability The ability record that has timed out while loading.
     */
    void HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &ability);

    /**
     * @brief Handles a foreground timeout event for a given ability record.
     *
     * @param ability The ability record that has timed out in the foreground.
     * @param state The state to transition the ability record to upon timeout (default is AbilityState::INITIAL).
     */
    void HandleForegroundTimeout(const std::shared_ptr<AbilityRecord> &ability,
        AbilityState state = AbilityState::INITIAL);

    /**
     * @brief Handles a timeout event and resumes the specified ability record.
     *
     * @param ability The ability record that has timed out and needs to be resumed.
     * @param state The state to transition the ability record to upon resumption (default is AbilityState::INITIAL).
     */
    void HandleTimeoutAndResumeAbility(const std::shared_ptr<AbilityRecord> &ability,
        AbilityState state = AbilityState::INITIAL);

    /**
     * @brief Moves a given ability record to the terminate list.
     *
     * @param ability The ability record to be moved to the terminate list.
     */
    void MoveToTerminateList(const std::shared_ptr<AbilityRecord> &ability);

    /**
     * @brief Delays the resumption of a timeout event for the specified caller ability.
     *
     * @param callerAbility The ability record that initiated the operation and whose timeout resumption is being
     * delayed.
     */
    void DelayedResumeTimeout(const std::shared_ptr<AbilityRecord> &callerAbility);

    /**
     * @brief Navigates back to the caller ability.
     *
     * @param callerAbility The ability record that initiated the operation and to which the system should navigate
     * back.
     */
    void BackToCaller(const std::shared_ptr<AbilityRecord> &callerAbility);

    // new version for call inner function.
    /**
     * @brief Completes the foreground transition for the specified ability record, indicating failure and updating
     * its state.
     *
     * @param abilityRecord The ability record for which the foreground transition is being completed.
     * @param state The new state to transition the ability record to upon failure.
     */
    void CompleteForegroundFailed(MissionAbilityRecordPtr abilityRecord, AbilityState state);

    /**
     * @brief Resolves an ability based on the provided target ability and ability request.
     *
     * @param targetAbility The target ability record to resolve.
     * @param abilityRequest The ability request that triggered the resolution.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int ResolveAbility(const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest);

    /**
     * @brief Retrieves an ability record by its element name.
     *
     * @param element The element name of the ability record to retrieve.
     * @return A shared pointer to the ability record, or a null pointer if not found.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByName(const AppExecFwk::ElementName &element);

    /**
     * @brief Retrieves an ability record by its element name from the current mission lists.
     *
     * @param element The element name of the ability record to retrieve.
     * @return A shared pointer to the ability record, or a null pointer if not found.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByNameFromCurrentMissionLists(
        const AppExecFwk::ElementName &element) const;

    /**
     * @brief Retrieves all ability records with the specified element name.
     *
     * @param element The element name to match.
     * @return A vector of shared pointers to the ability records that match the element name.
     */
    std::vector<std::shared_ptr<AbilityRecord>> GetAbilityRecordsByName(const AppExecFwk::ElementName &element);

    /**
     * @brief Calls an ability in a locked context, ensuring thread safety.
     *
     * @param abilityRequest The ability request to call.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int CallAbilityLocked(const AbilityRequest &abilityRequest);

    /**
     * @brief Updates the mission snapshot for the specified ability record.
     *
     * @param abilityRecord The ability record for which to update the mission snapshot.
     */
    void UpdateMissionSnapshot(const std::shared_ptr<AbilityRecord> &abilityRecord) const;

    /**
     * @brief Adds uninstall tags for the specified bundle name and UID.
     *
     * @param bundleName The bundle name for which to add uninstall tags.
     * @param uid The unique identifier of the user.
     */
    void AddUninstallTags(const std::string &bundleName, int32_t uid);

    /**
     * @brief Erases waiting ability records for the specified bundle name and UID.
     *
     * @param bundleName The bundle name for which to erase waiting ability records.
     * @param uid The unique identifier of the user.
     */
    void EraseWaitingAbility(const std::string &bundleName, int32_t uid);

    /**
     * @brief Removes a mission in a locked context, optionally excluding it from the mission list.
     *
     * @param missionId The unique identifier of the mission to remove.
     * @param excludeFromMissions A boolean indicating whether to exclude the mission from the mission list.
     */
    void RemoveMissionLocked(int32_t missionId, bool excludeFromMissions);

    /**
     * @brief Terminates the previous ability record for the specified ability record.
     *
     * @param abilityRecord The ability record for which to terminate the previous ability.
     */
    void TerminatePreviousAbility(MissionAbilityRecordPtr abilityRecord);

    /**
     * @brief Notifies that a mission has been created for the specified ability record.
     *
     * @param abilityRecord The ability record for which the mission has been created.
     */
    void NotifyMissionCreated(const std::shared_ptr<AbilityRecord> &abilityRecord) const;

    /**
     * @brief Checks if a mission is excluded from the mission list.
     *
     * @param mission The mission to check.
     * @return A boolean indicating whether the mission is excluded from the mission list.
     */
    bool IsExcludeFromMissions(const std::shared_ptr<Mission> &mission);

    /**
     * @brief Builds inner mission information based on the provided mission name and ability request.
     *
     * @param info A reference to the inner mission info structure to fill.
     * @param missionName The name of the mission.
     * @param abilityRequest The ability request that triggered the mission creation.
     */
    void BuildInnerMissionInfo(InnerMissionInfo &info, const std::string &missionName,
        const AbilityRequest &abilityRequest) const;

    /**
     * @brief Notifies the system to start a specified ability.
     *
     * @param request The ability request to start.
     * @param want The want object containing additional information about the ability to start.
     */
    void NotifyStartSpecifiedAbility(AbilityRequest &request, const AAFwk::Want &want);

    /**
     * @brief Notifies the system to restart a specified ability.
     *
     * @param request The ability request to restart.
     * @param token A remote object token associated with the ability to restart.
     */
    void NotifyRestartSpecifiedAbility(AbilityRequest &request, const sptr<IRemoteObject> &token);

    /**
     * @brief Processes preload actions for the specified ability record.
     *
     * @param record The ability record for which to process preload actions.
     */
    void ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const;

    /**
     * @brief Updates the launch reason for the specified ability record based on the ability request.
     *
     * @param abilityRequest The ability request that triggered the launch.
     * @param targetAbilityRecord A reference to the ability record for which to update the launch reason.
     * @return A boolean indicating whether the update was successful.
     */
    bool UpdateAbilityRecordLaunchReason(
        const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> targetAbilityRecord);

    /**
     * @brief Retrieves an alive ability record by its remote object token.
     *
     * @param token The remote object token of the ability record to retrieve.
     * @return A shared pointer to the alive ability record, or a null pointer if not found.
     */
    std::shared_ptr<AbilityRecord> GetAliveAbilityRecordByToken(const sptr<IRemoteObject> &token) const;

    /**
     * @brief Notifies the system of an ability token and associated ability request.
     *
     * @param token The remote object token of the ability.
     * @param abilityRequest The ability request associated with the token.
     */
    void NotifyAbilityToken(const sptr<IRemoteObject> &token, const AbilityRequest &abilityRequest);

    /**
     * @brief Notifies the system of the result of starting an ability.
     *
     * @param abilityRequest The ability request that was attempted.
     * @param result The result of the operation (e.g., success or error code).
     */
    void NotifyStartAbilityResult(const AbilityRequest &abilityRequest, int result);

    /**
     * @brief Moves a mission to the front in an inner context, handling caller and recent mission scenarios.
     *
     * @param missionId The unique identifier of the mission to move.
     * @param isCallerFromLauncher A boolean indicating whether the caller is from the launcher.
     * @param isRecent A boolean indicating whether the mission is recent.
     * @param callerAbility The ability record of the caller.
     * @param startOptions Optional start options for the ability.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int MoveMissionToFrontInner(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr,
        bool isNewWant = false);

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

    /**
     * @brief Retrieves an ability record by its remote object token in an internal context.
     *
     * @param token The remote object token of the ability record to retrieve.
     * @return A shared pointer to the ability record, or a null pointer if not found.
     */
    MissionAbilityRecordPtr GetAbilityRecordByTokenInner(const sptr<IRemoteObject> &token) const;

    /**
     * @brief Terminates an ability record internally, with additional parameters for result code and want.
     *
     * @param abilityRecord The ability record to terminate.
     * @param resultCode The result code to associate with the termination.
     * @param resultWant An optional want object to associate with the termination.
     * @param flag A boolean flag that may affect the termination behavior.
     * @return An integer indicating the result of the operation (e.g., success or error code).
     */
    int TerminateAbilityInner(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant, bool flag);

    /**
     * @brief Retrieves the mission ID associated with a given ability token in an internal context.
     *
     * @param token The remote object token of the ability.
     * @return The mission ID associated with the ability, or a default value if not found.
     */
    int32_t GetMissionIdByAbilityTokenInner(const sptr<IRemoteObject> &token);

    /**
     * @brief Retrieves an ability record from the terminate list by its remote object token in an internal context.
     *
     * @param token The remote object token of the ability record to retrieve.
     * @return A shared pointer to the ability record from the terminate list, or a null pointer if not found.
     */
    MissionAbilityRecordPtr GetAbilityFromTerminateListInner(const sptr<IRemoteObject> &token);

    /**
     * @brief Sets the last exit reason for the specified ability record.
     *
     * @param abilityRecord A reference to the ability record for which to set the last exit reason.
     */
    static void SetLastExitReason(std::shared_ptr<AbilityRecord> abilityRecord);
    void LoadLastExitReasonAsync(MissionAbilityRecordPtr abilityRecord);
    void SyncLoadExitReasonTask(int32_t abilityRecordId);

    /**
     * @brief Checks if the specified ability record is the last ability of its application.
     *
     * @param abilityRecord The ability record to check.
     * @return A boolean indicating whether the ability record is the last ability of its application.
     */
    bool IsAppLastAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * @brief Retrieves the MissionList object associated with a specific mission ID.
     *
     * @param missionId The ID of the mission for which to retrieve the MissionList.
     * @return A shared pointer to the MissionList object, or nullptr if not found.
     */
    std::shared_ptr<MissionList> GetMissionList(int32_t missionId);

    /**
     * @brief Prepares to clear a mission, ensuring it is in a state that allows safe removal.
     *
     * @param missionId The ID of the mission to be cleared.
     * @param mission A shared pointer to the Mission object to be cleared.
     * @return An integer indicating the result of the preparation operation (e.g., success or error code).
     */
    int PrepareClearMissionLocked(int missionId, const std::shared_ptr<Mission> &mission);

    /**
     * @brief Checks if a mission is ready to be terminated, based on internal prepare conditions.
     *
     * @param mission The mission to check.
     * @return A boolean indicating whether the mission is ready to be terminated.
     */
    bool CheckPrepareTerminateEnable(const std::shared_ptr<Mission> &mission);

    /**
     * @brief Retrieves the content and type ID associated with a given message ID.
     *
     * @param msgId The unique identifier of the message.
     * @param msgContent A reference to a string that will hold the message content.
     * @param typeId A reference to an integer that will hold the type ID.
     * @return A boolean indicating whether the retrieval was successful.
     */
    bool GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const;

    /**
     * @brief Sends a key event to the specified ability.
     *
     * @param abilityRequest The ability request associated with the key event.
     */
    void SendKeyEvent(const AbilityRequest &abilityRequest);

    bool isPrepareTerminateEnable_ = false;
    int userId_;
    // only manager the ability of standard in the default list
    std::shared_ptr<MissionList> defaultStandardList_;
    // only manager the ability of singleton in the default list for the fast search
    std::shared_ptr<MissionList> defaultSingleList_;
    std::shared_ptr<MissionList> launcherList_;
    std::shared_ptr<MissionListenerController> listenerController_;
    // launcher list is also in currentMissionLists_
    std::list<std::shared_ptr<MissionList>> currentMissionLists_;
    std::list<std::shared_ptr<AbilityRecord>> terminateAbilityList_;

    std::queue<AbilityRequest> waitingAbilityQueue_;
    mutable ffrt::mutex managerLock_;

    std::mutex exitReasonTaskMutex_;
    std::unordered_map<int32_t, ffrt::task_handle> exitReasonTasks_; // for sync querying exit-reason task
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H
