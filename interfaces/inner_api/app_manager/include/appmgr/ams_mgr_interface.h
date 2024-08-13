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

#ifndef OHOS_ABILITY_RUNTIME_AMS_MGR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_AMS_MGR_INTERFACE_H

#include "ability_debug_response_interface.h"
#include "ability_info.h"
#include "app_debug_listener_interface.h"
#include "app_record_id.h"
#include "application_info.h"
#include "configuration.h"
#include "iapp_state_callback.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "istart_specified_ability_response.h"
#include "running_process_info.h"

namespace OHOS {
namespace AppExecFwk {
class IAmsMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IAmsMgr");

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
        const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId) {};

    /**
     * TerminateAbility, call TerminateAbility() through the proxy object, terminate the token ability.
     *
     * @param token, token, he unique identification to terminate the ability.
     * @param clearMissionFlag, indicates whether terminate the ability when clearMission.
     * @return
     */
    virtual void TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag) = 0;

    /**
     * UpdateAbilityState, call UpdateAbilityState() through the proxy object, update the ability status.
     *
     * @param token, the unique identification to update the ability.
     * @param state, ability status that needs to be updated.
     * @return
     */
    virtual void UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state) = 0;

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    virtual void UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state) = 0;

    /**
     * RegisterAppStateCallback, call RegisterAppStateCallback() through the proxy object, register the callback.
     *
     * @param callback, Ams register the callback.
     * @return
     */
    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback> &callback) = 0;

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
        const int32_t visibility, const int32_t perceptibility, const int32_t connectionState) = 0;

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return
     */
    virtual void KillProcessByAbilityToken(const sptr<IRemoteObject> &token) = 0;

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the processes by userId.
     *
     * @param userId, the user id.
     * @return
     */
    virtual void KillProcessesByUserId(int32_t userId) = 0;

    virtual void KillProcessesByPids(std::vector<int32_t> &pids) {}

    virtual void AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken) {}

    /**
     * KillProcessWithAccount, call KillProcessWithAccount() through proxy object,
     * kill the process.
     *
     * @param bundleName, bundle name in Application record.
     * @param accountId, account ID.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int KillProcessWithAccount(
        const std::string &bundleName, const int accountId, const bool clearpagestack = false) = 0;

    /**
     * UpdateApplicationInfoInstalled, call UpdateApplicationInfoInstalled() through proxy object,
     * update the application info after new module installed.
     *
     * @param bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid) = 0;

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int KillApplication(const std::string &bundleName, const bool clearpagestack = false) = 0;

    /**
     * ForceKillApplication, call ForceKillApplication() through proxy object, force kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  userId, userId.
     * @param  appIndex, appIndex.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int ForceKillApplication(const std::string &bundleName, const int userId = -1, const int appIndex = 0) = 0;

    /**
     * KillProcessesByAccessTokenId, call KillProcessesByAccessTokenId() through proxy object,
     * force kill the application.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int KillProcessesByAccessTokenId(const uint32_t accessTokenId) = 0;

    /**
     * KillApplicationByUid, call KillApplicationByUid() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  userId, userId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int KillApplicationByUid(const std::string &bundleName, const int uid) = 0;

    /**
     * Kill the application self.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillApplicationSelf(const bool clearpagestack = false)
    {
        return ERR_OK;
    }

    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject> &token) = 0;

    virtual void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false) = 0;

    virtual void GetRunningProcessInfoByToken(
        const sptr<IRemoteObject> &token, OHOS::AppExecFwk::RunningProcessInfo &info) = 0;

    virtual void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) = 0;

    virtual void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0) = 0;

    virtual void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response) = 0;

    virtual void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0) = 0;

    virtual int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug) = 0;

    /**
     * Record process exit reason to appRunningRecord
     * @param pid pid
     * @param reason reason enum
     * @param exitMsg exitMsg
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg) = 0;

    /**
     * Set the current userId of appMgr.
     *
     * @param userId the user id.
     *
     * @return
     */
    virtual void SetCurrentUserId(const int32_t userId) = 0;

    /**
     * Get bundleName by pid.
     *
     * @param pid process id.
     * @param bundleName Output parameters, return bundleName.
     * @param uid Output parameters, return userId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid) = 0;

    /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener) = 0;

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener) = 0;

    /**
     * @brief Attach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t AttachAppDebug(const std::string &bundleName) = 0;

    /**
     * @brief Detach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t DetachAppDebug(const std::string &bundleName) = 0;

    /**
     * @brief Set app waiting debug mode.
     * @param bundleName The application bundle name.
     * @param isPersist The persist flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetAppWaitingDebug(const std::string &bundleName, bool isPersist) = 0;

    /**
     * @brief Cancel app waiting debug mode.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CancelAppWaitingDebug() = 0;

    /**
     * @brief Get waiting debug mode application.
     * @param debugInfoList The debug info list, including bundle name and persist flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetWaitingDebugApp(std::vector<std::string> &debugInfoList) = 0;

    /**
     * @brief Determine whether it is a waiting debug application based on the bundle name.
     * @return Returns true if it is a waiting debug application, otherwise it returns false.
     */
    virtual bool IsWaitingDebugApp(const std::string &bundleName) = 0;

    /**
     * @brief Clear non persist waiting debug flag.
     */
    virtual void ClearNonPersistWaitingDebugFlag() = 0;

    /**
     * @brief Registering ability debug mode response.
     * @param response Response for ability debug object.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response) = 0;

    /**
     * @brief Determine whether it is an attachment debug application based on the bundle name.
     * @param bundleName The application bundle name.
     * @return Returns true if it is an attach debug application, otherwise it returns false.
     */
    virtual bool IsAttachDebug(const std::string &bundleName) = 0;

    /**
     * @brief Set resident process enable status.
     * @param bundleName The application bundle name.
     * @param enable The current updated enable status.
     */
    virtual void SetKeepAliveEnableState(const std::string &bundleName, bool enable) {};

    /**
     * To clear the process by ability token.
     *
     * @param token the unique identification to the ability.
     */
    virtual void ClearProcessByToken(sptr<IRemoteObject> token) {}

    /**
     * whether memory size is sufficent.
     * @return Returns true is sufficent memory size, others return false.
     */
    virtual bool IsMemorySizeSufficent() = 0;

    /**
     * Notifies that one ability is attached to status bar.
     *
     * @param token the token of the abilityRecord that is attached to status bar.
     */
    virtual void AttachedToStatusBar(const sptr<IRemoteObject> &token) {}

    /**
     * Temporarily block the process cache feature.
     *
     * @param pids the pids of the processes that should be blocked.
     */
    virtual void BlockProcessCacheByPids(const std::vector<int32_t> &pids) {}

    /**
     * Request to clean uiability from user.
     *
     * @param token the token of ability.
     * @return Returns true if clean success, others return false.
     */
    virtual bool CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
    {
        return false;
    }

    /**
     * whether killed for upgrade web.
     *
     * @param bundleName the bundle name is killed for upgrade web.
     * @return Returns true is killed for upgrade web, others return false.
     */
    virtual bool IsKilledForUpgradeWeb(const std::string &bundleName)
    {
        return true;
    }

    /**
     * whether the abilities of process specified by pid type only UIAbility.
     * @return Returns true is only UIAbility, otherwise return false
     */
    virtual bool IsProcessContainsOnlyUIAbility(const pid_t pid)
    {
        return false;
    }

    enum class Message {
        LOAD_ABILITY = 0,
        TERMINATE_ABILITY,
        UPDATE_ABILITY_STATE,
        UPDATE_EXTENSION_STATE,
        REGISTER_APP_STATE_CALLBACK,
        ABILITY_BEHAVIOR_ANALYSIS,
        KILL_PEOCESS_BY_ABILITY_TOKEN,
        KILL_PROCESSES_BY_USERID,
        KILL_PROCESS_WITH_ACCOUNT,
        KILL_APPLICATION,
        ABILITY_ATTACH_TIMEOUT,
        PREPARE_TERMINATE_ABILITY,
        KILL_APPLICATION_BYUID,
        GET_RUNNING_PROCESS_INFO_BY_TOKEN,
        START_SPECIFIED_ABILITY,
        REGISTER_START_SPECIFIED_ABILITY_RESPONSE,
        UPDATE_CONFIGURATION,
        GET_CONFIGURATION,
        GET_APPLICATION_INFO_BY_PROCESS_ID,
        KILL_APPLICATION_SELF,
        UPDATE_APPLICATION_INFO_INSTALLED,
        SET_CURRENT_USER_ID,
        Get_BUNDLE_NAME_BY_PID,
        SET_ABILITY_FOREGROUNDING_FLAG,
        REGISTER_APP_DEBUG_LISTENER,
        UNREGISTER_APP_DEBUG_LISTENER,
        ATTACH_APP_DEBUG,
        DETACH_APP_DEBUG,
        SET_APP_WAITING_DEBUG,
        CANCEL_APP_WAITING_DEBUG,
        GET_WAITING_DEBUG_APP,
        IS_WAITING_DEBUG_APP,
        CLEAR_NON_PERSIST_WAITING_DEBUG_FLAG,
        REGISTER_ABILITY_DEBUG_RESPONSE,
        IS_ATTACH_DEBUG,
        START_SPECIFIED_PROCESS,
        CLEAR_PROCESS_BY_TOKEN,
        REGISTER_ABILITY_MS_DELEGATE,
        KILL_PROCESSES_BY_PIDS,
        ATTACH_PID_TO_PARENT,
        IS_MEMORY_SIZE_SUFFICIENT,
        NOTIFY_APP_MGR_RECORD_EXIT_REASON,
        SET_KEEP_ALIVE_ENABLE_STATE,
        ATTACHED_TO_STATUS_BAR,
        BLOCK_PROCESS_CACHE_BY_PIDS,
        IS_KILLED_FOR_UPGRADE_WEB,
        IS_PROCESS_CONTAINS_ONLY_UI_EXTENSION,
        FORCE_KILL_APPLICATION,
        CLEAN_UIABILITY_BY_USER_REQUEST,
        FORCE_KILL_APPLICATION_BY_ACCESS_TOKEN_ID = 49,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_AMS_MGR_INTERFACE_H
