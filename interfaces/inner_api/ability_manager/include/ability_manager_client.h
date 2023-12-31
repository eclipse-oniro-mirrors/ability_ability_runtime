/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H

#include <mutex>

#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "ability_scheduler_interface.h"
#include "ability_manager_interface.h"
#include "snapshot.h"
#include "want.h"

#include "iremote_object.h"
#include "system_memory_attr.h"
#include "ui_extension_window_command.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityManagerClient
 * AbilityManagerClient is used to access ability manager services.
 */
class AbilityManagerClient {
public:
    AbilityManagerClient();
    virtual ~AbilityManagerClient();
    static std::shared_ptr<AbilityManagerClient> GetInstance();

    /**
     * AttachAbilityThread, ability call this interface after loaded.
     *
     * @param scheduler,.the interface handler of kit ability.
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData);

    /**
     * ScheduleConnectAbilityDone, service ability call this interface while session was connected.
     *
     * @param token,.service ability's token.
     * @param remoteObject,.the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ScheduleConnectAbilityDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject);

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token);

    /**
     * ScheduleCommandAbilityDone, service ability call this interface while session was commanded.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token);

    ErrCode ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd);

    /**
     * Get top ability.
     *
     * @return Returns front desk focus ability elementName.
     */
    AppExecFwk::ElementName GetTopAbility();

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want Ability want.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(const Want &want, int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want Ability want.
     * @param callerToken caller ability token.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param requestCode the resultCode of the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want Ability want.
     * @param callerToken caller ability token.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbilityAsCaller(
            const Want &want,
            const sptr<IRemoteObject> &callerToken,
            int requestCode = DEFAULT_INVAL_VALUE,
            int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want Indicates the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbilityAsCaller(
            const Want &want,
            const StartOptions &startOptions,
            const sptr<IRemoteObject> &callerToken,
            int requestCode = DEFAULT_INVAL_VALUE,
            int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED);

    /**
     * Start ui extension ability with extension session info, send extension session info to ability manager service.
     *
     * @param extensionSessionInfo the extension session info of the ability to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartUIExtensionAbility(
        const sptr<SessionInfo> &extensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start ui ability with want, send want to ability manager service.
     *
     * @param sessionInfo the session info of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo);

    /**
     * Stop extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to stop.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be stopped.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StopExtensionAbility(
        const Want& want,
        const sptr<IRemoteObject>& callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED);

    /**
     * TerminateAbility with want, return want from ability manager service.
     *
     * @param token Ability token.
     * @param resultCode resultCode.
     * @param Want Ability want returned.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant);

    /**
     * TerminateUIExtensionAbility with want, return want from ability manager service.
     *
     * @param extensionSessionInfo the extension session info of the ability to terminate.
     * @param resultCode resultCode.
     * @param Want Ability want returned.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        int resultCode = DEFAULT_INVAL_VALUE, const Want *resultWant = nullptr);

    /**
     *  CloseUIAbilityBySCB, close the special ability by scb.
     *
     * @param sessionInfo the session info of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo);

    /**
     * SendResultToAbility with want, return resultWant from ability manager service.
     *
     * @param requestCode requestCode.
     * @param resultCode resultCode.
     * @param resultWant Ability want returned.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SendResultToAbility(int requestCode, int resultCode, Want& resultWant);

/**
     * MoveAbilityToBackground.
     *
     * @param token Ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveAbilityToBackground(const sptr<IRemoteObject> &token);

    /**
     * CloseAbility with want, return want from ability manager service.
     *
     * @param token Ability token.
     * @param resultCode resultCode.
     * @param Want Ability want returned.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr);

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param callerToken, caller ability token.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode TerminateAbility(const sptr<IRemoteObject> &callerToken, int requestCode);

    /**
     * Destroys this Service ability if the number of times it
     * has been started equals the number represented by
     * the given startId.
     *
     * @param token ability's token.
     * @param startId is incremented by 1 every time this ability is started.
     * @return Returns true if the startId matches the number of startup times
     * and this Service ability will be destroyed; returns false otherwise.
     */
    ErrCode TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId);

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, ability token.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false);

    /**
     * MinimizeUIExtensionAbility, minimize the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to minimize.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, bool fromUser = false);

    /**
     * MinimizeUIAbilityBySCB, minimize the special ability by scb.
     *
     * @param sessionInfo the session info of the ability to minimize.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MinimizeUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo);

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectAbility(const Want &want, const sptr<IAbilityConnection> &connect, int32_t userId);

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Connect data share extension ability.
     *
     * @param want, special want for the data share extension ability.
     * @param connect, callback used to notify caller the result of connecting or disconnecting.
     * @param userId, the extension runs in.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectDataShareExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Connect extension ability.
     *
     * @param want, special want for the extension ability.
     * @param connect, callback used to notify caller the result of connecting or disconnecting.
     * @param userId, the extension runs in.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Connect ui extension ability.
     *
     * @param want, special want for the ui extension ability.
     * @param connect, callback used to notify caller the result of connecting or disconnecting.
     * @param sessionInfo the extension session info of the ability to connect.
     * @param userId, the extension runs in.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<SessionInfo> &sessionInfo, int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * DisconnectAbility, disconnect session with service ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DisconnectAbility(const sptr<IAbilityConnection> &connect);

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param tryBind, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    sptr<IAbilityScheduler> AcquireDataAbility(const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken);

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityToken, specifies the data ability that will be released.
     * @param callerToken, specifies the caller ability token.
     * @return returns ERR_OK if succeeded, or error codes for failed.
     */
    ErrCode ReleaseDataAbility(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken);

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DumpState(const std::string &args, std::vector<std::string> &state);
    ErrCode DumpSysState(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID);
    /**
     * Connect ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode Connect();

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @param token ability's token.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    ErrCode StopServiceAbility(const Want &want, const sptr<IRemoteObject> &token = nullptr);

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode KillProcess(const std::string &bundleName);

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Force ability timeout.
     *
     * @param abilityName.
     * @param state. ability lifecycle state.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ForceTimeoutForTest(const std::string &abilityName, const std::string &state);
    #endif

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ClearUpApplicationData(const std::string &bundleName);

    /**
     * ContinueMission, continue ability from mission center.
     *
     * @param srcDeviceId, origin deviceId.
     * @param dstDeviceId, target deviceId.
     * @param missionId, indicates which ability to continue.
     * @param callBack, notify result back.
     * @param wantParams, extended params.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject> &callback, AAFwk::WantParams &wantParams);

    /**
     * ContinueMission, continue ability from mission center.
     *
     * @param srcDeviceId, origin deviceId.
     * @param dstDeviceId, target deviceId.
     * @param bundleName, indicates which bundleName to continue.
     * @param callBack, notify result back.
     * @param wantParams, extended params.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        const std::string &bundleName, const sptr<IRemoteObject> &callback, AAFwk::WantParams &wantParams);

    /**
     * start continuation.
     * @param want, used to start a ability.
     * @param abilityToken, ability token.
     * @param status, continue status.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status);

    /**
     * notify continuation complete to dms.
     * @param deviceId, source device which start a continuation.
     * @param sessionId, represent a continuaion.
     * @param isSuccess, continuation result.
     * @return
     */
    void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess);

    /**
     * ContinueMission, continue ability from mission center.
     * @param deviceId, target deviceId.
     * @param missionId, indicates which ability to continue.
     * @param versionCode, version of the remote target ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode);

    /**
     * notify continuation result to application.
     * @param missionId, indicates which ability to notify.
     * @param result, continuation result.
     * @return
     */
    ErrCode NotifyContinuationResult(int32_t missionId, int32_t result);

    /**
     * @brief Lock specified mission.
     * @param missionId The id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode LockMissionForCleanup(int32_t missionId);

    /**
     * @brief Unlock specified mission.
     * @param missionId The id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnlockMissionForCleanup(int32_t missionId);

    /**
     * @brief Register mission listener to ams.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterMissionListener(const sptr<IMissionListener> &listener);

    /**
     * @brief UnRegister mission listener from ams.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnRegisterMissionListener(const sptr<IMissionListener> &listener);

    /**
     * @brief Register mission listener to ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterMissionListener(const std::string &deviceId, const sptr<IRemoteMissionListener> &listener);

    /**
     * @brief Register mission listener to ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterOnListener(const std::string &type, const sptr<IRemoteOnListener> &listener);

    /**
     * @brief Register mission listener to ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterOffListener(const std::string &type, const sptr<IRemoteOnListener> &listener);

    /**
     * @brief UnRegister mission listener from ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnRegisterMissionListener(const std::string &deviceId, const sptr<IRemoteMissionListener> &listener);

    /**
     * @brief Get mission infos from ams.
     * @param deviceId local or remote deviceid.
     * @param numMax max number of missions.
     * @param missionInfos mission info result.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionInfos(const std::string &deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos);

    /**
     * @brief Get mission info by id.
     * @param deviceId local or remote deviceid.
     * @param missionId Id of target mission.
     * @param missionInfo mision info of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo);

    /**
     * @brief Get the Mission Snapshot Info object
     * @param deviceId local or remote deviceid.
     * @param missionId Id of target mission.
     * @param snapshot snapshot of target mission.
     * @param isLowResolution get low resolution snapshot.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution = false);

    /**
     * @brief Clean mission by id.
     * @param missionId Id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CleanMission(int32_t missionId);

    /**
     * @brief Clean all missions in system.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CleanAllMissions();

    /**
     * @brief Move a mission to front.
     * @param missionId Id of target mission.
     * @param startOptions Special startOptions for target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionToFront(int32_t missionId);
    ErrCode MoveMissionToFront(int32_t missionId, const StartOptions &startOptions);

    /**
     * Move missions to front
     * @param missionIds Ids of target missions
     * @param topMissionId Indicate which mission will be moved to top, if set to -1, missions' order won't change
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId);

    /**
     * Move missions to background
     * @param missionIds Ids of target missions
     * @param result The result of move missions to background, and the array is sorted by zOrder
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result);

    /**
     * @brief Get mission id by ability token.
     *
     * @param token ability token.
     * @param missionId output mission id.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionIdByToken(const sptr<IRemoteObject> &token, int32_t &missionId);

    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect);

    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param accountId Indicates the account to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callToken, int32_t accountId = DEFAULT_INVAL_VALUE);

    /**
     * CallRequestDone, after invoke callRequest, ability will call this interface to return callee.
     *
     * @param token, ability's token.
     * @param callStub, ability's callee.
     */
    void CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub);

    /**
     * Get ability token by connect.
     *
     * @param token The token of ability.
     * @param callStub The callee object.
     */
    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token);

    /**
     * Release the call between Ability, disconnect session with common ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ReleaseCall(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element);

    /**
     * @brief Get the ability running information.
     *
     * @param info Ability running information.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info);

    /**
     * @brief Get the extension running information.
     *
     * @param upperLimit The maximum limit of information wish to get.
     * @param info Extension running information.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info);

    /**
     * @brief Get running process information.
     *
     * @param info Running process information.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info);

    /**
     * Start synchronizing remote device mission
     * @param devId, deviceId.
     * @param fixConflict, resolve synchronizing conflicts flag.
     * @param tag, call tag.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag);

    /**
     * Stop synchronizing remote device mission
     * @param devId, deviceId.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StopSyncRemoteMissions(const std::string &devId);

    /**
     * @brief start user.
     * @param accountId accountId.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartUser(int accountId);

    /**
     * @brief stop user.
     * @param accountId accountId.
     * @param callback callback.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StopUser(int accountId, const sptr<IStopUserCallback> &callback);

    /**
     * @brief Register the snapshot handler
     * @param handler snapshot handler
     * @return ErrCode Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler);

    /**
     * PrepareTerminateAbility with want, if terminate, return want from ability manager service.
     *
     * @param token Ability token.
     * @param callback callback.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode PrepareTerminateAbility(const sptr<IRemoteObject> &token, sptr<IPrepareTerminateCallback> &callback);
#ifdef SUPPORT_GRAPHICS
    /**
     * Set mission label of this ability.
     *
     * @param abilityToken Indidate token of ability.
     * @param label Indidate the label showed of the ability in recent missions.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label);

    /**
     * Set mission icon of this ability.
     *
     * @param abilityToken Indidate token of ability.
     * @param icon Indidate the icon showed of the ability in recent missions.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionIcon(const sptr<IRemoteObject> &abilityToken,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon);

    /**
     * Register the WindowManagerService handler
     *
     * @param handler Indidate handler of WindowManagerService.
     * @return ErrCode Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler);

    /**
     * WindowManager notification AbilityManager after the first frame is drawn.
     *
     * @param abilityToken Indidate token of ability.
     */
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken);

    /**
     * Called to update mission snapshot.
     * @param token The target ability.
     * @param pixelMap The snapshot.
     */
    void UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &pixelMap);
#endif

    /**
     * @brief start user test.
     * @param want the want of the ability user test to start.
     * @param observer test observer callback.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartUserTest(const Want &want, const sptr<IRemoteObject> &observer);

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName);

     /**
     * GetTopAbility, get the token of top ability.
     *
     * @param token, the token of top ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetTopAbility(sptr<IRemoteObject> &token);

    /**
     * DelegatorDoAbilityForeground, the delegator calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token);

    /**
     * DelegatorDoAbilityBackground, the delegator calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token);

   /**
     * Calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag);

    /**
     * Calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag);

    /**
     * Set ability controller.
     *
     * @param abilityController, The ability controller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest);

    /**
     * Send not response process ID to ability manager service.
     *
     * @param pid The not response process ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SendANRProcessID(int pid);

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode BlockAmsService();

    /**
     * Block ability.
     *
     * @param abilityRecordId The Ability Record Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode BlockAbility(int32_t abilityRecordId);

    /**
     * Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode BlockAppService();
    #endif

    /**
     * Free install ability from remote DMS.
     *
     * @param want Ability want.
     * @param callback Callback used to notify free install result.
     * @param userId User ID.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback, int32_t userId,
        int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * Called when client complete dump.
     *
     * @param infos The dump info.
     * @param callerToken The caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken);

    /**
     * Called to update mission snapshot.
     * @param token The target ability.
     */
    void UpdateMissionSnapShot(const sptr<IRemoteObject>& token);

    /**
     * @brief Enable recover ability.
     *
     * @param token Ability identify.
     */
    void EnableRecoverAbility(const sptr<IRemoteObject>& token);

    /**
     * @brief Schedule recovery ability.
     *
     * @param token Ability identify.
     * @param reason See AppExecFwk::StateReason.
     * @param want Want information.
     */
    void ScheduleRecoverAbility(const sptr<IRemoteObject> &token, int32_t reason, const Want *want = nullptr);

    /**
     * @brief Add free install observer.
     *
     * @param observer Free install observer.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer);

    /**
     * Called to verify that the MissionId is valid.
     * @param missionIds Query mission list.
     * @param results Output parameters, return results up to 20 query results.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds, std::vector<MissionVaildResult> &results);

    /**
     * Query whether the application of the specified PID and UID has been granted a certain permission
     * @param permission
     * @param pid Process id
     * @param uid
     * @return Returns ERR_OK if the current process has the permission, others on failure.
     */
    ErrCode VerifyPermission(const std::string &permission, int pid, int uid);

    /**
     * Acquire the shared data.
     * @param missionId The missionId of Target ability.
     * @param The IAcquireShareDataCallback object.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AcquireShareData(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData);

    /**
     * Notify sharing data finished.
     * @param resultCode The result of sharing data.
     * @param uniqueId The uniqueId from request object.
     * @param wantParam The params of acquiring sharing data from target ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ShareDataDone(
        const sptr<IRemoteObject> &token, const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam);

    /**
     * Request dialog service with want, send want to ability manager service.
     *
     * @param want target component.
     * @param callerToken caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RequestDialogService(
        const Want &want,
        const sptr<IRemoteObject> &callerToken);

    /**
     * Force app exit and record exit reason.
     * @param pid Process id .
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ForceExitApp(const int32_t pid, Reason exitReason);

    /**
     * Record app exit reason.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RecordAppExitReason(Reason exitReason);

    /**
     * Set rootSceneSession by SCB.
     *
     * @param rootSceneSession Indicates root scene session of SCB.
     */
    void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession);

    /**
     * Call UIAbility by SCB.
     *
     * @param sessionInfo the session info of the ability to be called.
     */
    void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo);

    /**
     * Start specified ability by SCB.
     *
     * @param want Want information.
     */
    void StartSpecifiedAbilityBySCB(const Want &want);

    /**
     * Set sessionManagerService
     * @param sessionManagerService the point of sessionManagerService.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetSessionManagerService(const sptr<IRemoteObject> &sessionManagerService);

    /**
     * Get sessionManagerService
     *
     * @return returns the SessionManagerService object, or nullptr for failed.
     */
    sptr<IRemoteObject> GetSessionManagerService();

    ErrCode ReportDrawnCompleted(const sptr<IRemoteObject> &token);

private:
    class AbilityMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        AbilityMgrDeathRecipient() = default;
        ~AbilityMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(AbilityMgrDeathRecipient);
    };

    sptr<IAbilityManager> GetAbilityManager();
    void ResetProxy(const wptr<IRemoteObject>& remote);
    void HandleDlpApp(Want &want);

    static std::recursive_mutex mutex_;
    static std::shared_ptr<AbilityManagerClient> instance_;
    sptr<IAbilityManager> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
