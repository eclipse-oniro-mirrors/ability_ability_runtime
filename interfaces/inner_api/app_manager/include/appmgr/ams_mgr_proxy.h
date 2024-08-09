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

#ifndef OHOS_ABILITY_RUNTIME_AMS_MGR_PROXY_H
#define OHOS_ABILITY_RUNTIME_AMS_MGR_PROXY_H

#include "ams_mgr_interface.h"
#include "app_debug_listener_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class AmsMgrProxy : public IRemoteProxy<IAmsMgr> {
public:
    explicit AmsMgrProxy(const sptr<IRemoteObject> &impl);
    virtual ~AmsMgrProxy() = default;

    /**
     * LoadAbility, call LoadAbility() through proxy project, load the ability that needed to be started.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param abilityInfo, the ability information.
     * @param appInfo, the app information.
     * @return
     */
    virtual void LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
        const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId) override;

    /**
     * TerminateAbility, call TerminateAbility() through the proxy object, terminate the token ability.
     *
     * @param token, token, he unique identification to terminate the ability.
     * @param clearMissionFlag, indicates whether terminate the ability when clearMission.
     * @return
     */
    virtual void TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag) override;

    /**
     * UpdateAbilityState, call UpdateAbilityState() through the proxy object, update the ability status.
     *
     * @param token, the unique identification to update the ability.
     * @param state, ability status that needs to be updated.
     * @return
     */
    virtual void UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state) override;

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    virtual void UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state) override;

    /**
     * RegisterAppStateCallback, call RegisterAppStateCallback() through the proxy object, register the callback.
     *
     * @param callback, Ams register the callback.
     * @return
     */
    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback> &callback) override;

    /**
     * AbilityBehaviorAnalysis,call AbilityBehaviorAnalysis() through the proxy object,
     * ability behavior analysis assistant process optimization.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param visibility, the visibility information about windows info.
     * @param perceptibility, the Perceptibility information about windows info.
     * @param connectionState, the service ability connection state.
     * @return
     */
    virtual void AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const int32_t visibility, const int32_t perceptibility, const int32_t connectionState) override;

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return
     */
    virtual void KillProcessByAbilityToken(const sptr<IRemoteObject> &token) override;

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the processes by userId.
     *
     * @param userId, the user id.
     * @return
     */
    virtual void KillProcessesByUserId(int32_t userId) override;

    virtual void KillProcessesByPids(std::vector<int32_t> &pids) override;

    virtual void AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken) override;

    /**
     * KillProcessWithAccount, call KillProcessWithAccount() through proxy object,
     * kill the process.
     *
     * @param bundleName, bundle name in Application record.
     * @param accountId, account ID.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillProcessWithAccount(
        const std::string &bundleName, const int accountId, const bool clearpagestack = false) override;

    /**
     * UpdateApplicationInfoInstalled, call UpdateApplicationInfoInstalled() through proxy object,
     * update the application info after new module installed.
     *
     * @param bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid) override;

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillApplication(const std::string &bundleName, const bool clearpagestack = false) override;

    /**
     * ForceKillApplication, call ForceKillApplication() through proxy object, force kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  userId, userId.
     * @param  appIndex, appIndex.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int ForceKillApplication(const std::string &bundleName, const int userId = -1,
        const int appIndex = 0) override;

    /**
     * KillProcessesByAccessTokenId, call KillProcessesByAccessTokenId() through proxy object,
     * force kill the application.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int KillProcessesByAccessTokenId(const uint32_t accessTokenId) override;

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillApplicationByUid(const std::string &bundleName, const int uid) override;

    virtual int KillApplicationSelf(const bool clearpagestack = false) override;

    virtual int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application,
        bool &debug) override;

    virtual int32_t NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg) override;

    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject> &token) override;

    virtual void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false) override;

    void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info) override;

    /**
     * Set AbilityForegroundingFlag of an app-record to true.
     *
     * @param pid, pid.
     *
     */
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) override;

    virtual void StartSpecifiedAbility(
        const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo, int32_t requestId = 0) override;

    virtual void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response) override;

    virtual void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0) override;

    virtual void SetCurrentUserId(const int32_t userId) override;

    virtual int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid) override;

    /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener) override;

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener) override;

    /**
     * @brief Attach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t AttachAppDebug(const std::string &bundleName) override;

    /**
     * @brief Detach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DetachAppDebug(const std::string &bundleName) override;

    /**
     * @brief Set app waiting debug mode.
     * @param bundleName The application bundle name.
     * @param isPersist The persist flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetAppWaitingDebug(const std::string &bundleName, bool isPersist) override;

    /**
     * @brief Cancel app waiting debug mode.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelAppWaitingDebug() override;

    /**
     * @brief Get waiting debug mode application.
     * @param debugInfoList The debug info list, including bundle name and persist flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetWaitingDebugApp(std::vector<std::string> &debugInfoList) override;

    /**
     * @brief Determine whether it is a waiting debug application based on the bundle name.
     * @return Returns true if it is a waiting debug application, otherwise it returns false.
     */
    bool IsWaitingDebugApp(const std::string &bundleName) override;

    /**
     * @brief Clear non persist waiting debug flag.
     */
    void ClearNonPersistWaitingDebugFlag() override;

    /**
     * @brief Registering ability debug mode response.
     * @param response Response for ability debug object.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response) override;

    /**
     * @brief Determine whether it is an attachment debug application based on the bundle name.
     * @param bundleName The application bundle name.
     * @return Returns true if it is an attach debug application, otherwise it returns false.
     */
    bool IsAttachDebug(const std::string &bundleName) override;

    /**
     * @brief Set resident process enable status.
     * @param bundleName The application bundle name.
     * @param enable The current updated enable status.
     */
    void SetKeepAliveEnableState(const std::string &bundleName, bool enable) override;

    /**
     * To clear the process by ability token.
     *
     * @param token the unique identification to the ability.
     */
    virtual void ClearProcessByToken(sptr<IRemoteObject> token) override;

    /**
     * whether memory size is sufficent.
     * @return Returns true is sufficent memory size, others return false.
     */
    virtual bool IsMemorySizeSufficent() override;

    /**
     * Notifies that one ability is attached to status bar.
     *
     * @param token the token of the abilityRecord that is attached to status bar.
     */
    virtual void AttachedToStatusBar(const sptr<IRemoteObject> &token) override;

    /**
     * Temporarily block the process cache feature.
     *
     * @param pids the pids of the processes that should be blocked.
     */
    virtual void BlockProcessCacheByPids(const std::vector<int32_t> &pids) override;

    /**
     * Request to clean uiability from user.
     *
     * @param token the token of ability.
     * @return Returns true if clean success, others return false.
     */
    virtual bool CleanAbilityByUserRequest(const sptr<IRemoteObject> &token) override;

    /**
     * whether killed for upgrade web.
     *
     * @param bundleName the bundle name is killed for upgrade web.
     * @return Returns true is killed for upgrade web, others return false.
     */
    virtual bool IsKilledForUpgradeWeb(const std::string &bundleName) override;

    /**
     * whether the abilities of process specified by pid type only UIAbility.
     * @return Returns true is only UIAbility, otherwise return false
     */
    virtual bool IsProcessContainsOnlyUIAbility(const pid_t pid) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);

private:
    static inline BrokerDelegator<AmsMgrProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_AMS_MGR_PROXY_H
