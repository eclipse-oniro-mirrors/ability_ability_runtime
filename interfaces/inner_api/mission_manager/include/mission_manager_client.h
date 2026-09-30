/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_MISSION_MANAGER_CLIENT_H

#include <mutex>

#include "iremote_object.h"
#include "mission_manager_interface.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class MissionManagerClient
 * MissionManagerClient is used to access ability manager services.
 */
class MissionManagerClient {
public:
    MissionManagerClient() = default;
    virtual ~MissionManagerClient() = default;
    static MissionManagerClient& GetInstance();

    ErrCode Release();

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
        sptr<IRemoteObject> callback, AAFwk::WantParams &wantParams);

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
    ErrCode ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo, const sptr<IRemoteObject> &callback);

    /**
     * @brief Lock specified mission.
     * @param missionId The id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode LockMissionForCleanup(int32_t missionId, int32_t userId = -1);

    /**
     * @brief Unlock specified mission.
     * @param missionId The id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnlockMissionForCleanup(int32_t missionId, int32_t userId = -1);

        /**
     * @brief Register mission listener to ability mgr.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterMissionListener(sptr<IMissionListener> listener, int32_t userId = -1);

    /**
     * @brief UnRegister mission listener from ability mgr.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnRegisterMissionListener(sptr<IMissionListener> listener, int32_t userId = -1);

    /**
     * @brief Register mission listener to ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterMissionListener(const std::string &deviceId, sptr<IRemoteMissionListener> listener);

    /**
     * @brief UnRegister mission listener from ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnRegisterMissionListener(const std::string &deviceId, sptr<IRemoteMissionListener> listener);

    /**
     * @brief Get mission infos from ability mgr.
     * @param deviceId local or remote deviceId.
     * @param numMax max number of missions.
     * @param missionInfos mission info result.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionInfos(const std::string &deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos,
        int32_t userId = -1);

    /**
     * @brief Get mission info by id.
     * @param deviceId local or remote deviceId.
     * @param missionId Id of target mission.
     * @param missionInfo mission info of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo,
        int32_t userId = -1);

    /**
     * @brief Get the Mission Snapshot Info object
     * @param deviceId local or remote deviceId.
     * @param missionId Id of target mission.
     * @param snapshot snapshot of target mission.
     * @param isLowResolution get low resolution snapshot.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionSnapshot(const std::string &deviceId, int32_t missionId,
        MissionSnapshot &snapshot, bool isLowResolution = false, int32_t userId = -1);

    /**
     * @brief Clean mission by id.
     * @param missionId Id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CleanMission(int32_t missionId, int32_t userId = -1);

    /**
     * @brief Clean all missions in system.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CleanAllMissions(int32_t userId = -1);

    /**
     * @brief Move a mission to front.
     * @param missionId Id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionToFront(int32_t missionId);

        /**
     * @brief Move a mission to front.
     * @param missionId Id of target mission.
     * @param startOptions Special startOptions for target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionToFront(int32_t missionId, const StartOptions &startOptions);

    /**
     * Move missions to front
     * @param missionIds Ids of target missions
     * @param topMissionId Indicate which mission will be moved to top, if set to -1, missions' order won't change
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionsToForeground(const std::vector<int32_t> &missionIds, int32_t topMissionId,
        int32_t userId = -1);

    /**
     * Move missions to background
     * @param missionIds Ids of target missions
     * @param result The result of move missions to background, and the array is sorted by zOrder
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionsToBackground(const std::vector<int32_t> &missionIds, std::vector<int32_t> &result,
        int32_t userId = -1);

    /**
     * @brief Get mission id by ability token.
     *
     * @param token ability token.
     * @param missionId output mission id.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionIdByToken(sptr<IRemoteObject> token, int32_t &missionId);

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
     * Set mission continue state of this ability.
     *
     * @param token Indicate token of ability.
     * @param state the mission continuation state of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionContinueState(sptr<IRemoteObject> token, const AAFwk::ContinueState &state,
        sptr<IRemoteObject> sessionToken, int32_t userId = -1);

#ifdef SUPPORT_SCREEN
    /**
     * Set mission label of this ability.
     *
     * @param abilityToken Indicate token of ability.
     * @param label Indicate the label showed of the ability in recent missions.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionLabel(sptr<IRemoteObject> abilityToken, const std::string &label, int32_t userId = -1);

    /**
     * Set mission icon of this ability.
     *
     * @param abilityToken Indicate token of ability.
     * @param icon Indicate the icon showed of the ability in recent missions.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionIcon(sptr<IRemoteObject> abilityToken,
        std::shared_ptr<OHOS::Media::PixelMap> icon, int32_t userId = -1);

    /**
     * Called to update mission snapshot.
     * @param token The target ability.
     * @param pixelMap The snapshot.
     */
    void UpdateMissionSnapShot(sptr<IRemoteObject> token,
        std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
#endif

    /**
     * Called to verify that the MissionId is valid.
     * @param missionIds Query mission list.
     * @param results Output parameters, return results up to 20 query results.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results,
        int32_t userId = -1);

    /**
     * Open atomic service window prior to finishing free install.
     *
     * @param bundleName, the bundle name of the atomic service.
     * @param moduleName, the module name of the atomic service.
     * @param abilityName, the ability name of the atomic service.
     * @param startTime, the starting time of the free install task.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t PreStartMission(const std::string &bundleName, const std::string &moduleName,
        const std::string &abilityName, const std::string &startTime);

    /**
     * Terminate process by bundleName.
     *
     * @param missionId, The mission id of the UIAbility need to be terminated.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode TerminateMission(int32_t missionId, int32_t userId = -1);
private:
    class MissionMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        MissionMgrDeathRecipient() = default;
        ~MissionMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(MissionMgrDeathRecipient);
    };

    sptr<IMissionManager> GetMissionManager();
    void Connect();
    void ResetProxy(const wptr<IRemoteObject> &remote);
    ErrCode RemoveDeathRecipient();

    std::mutex mutex_;
    sptr<IMissionManager> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MISSION_MANAGER_CLIENT_H
