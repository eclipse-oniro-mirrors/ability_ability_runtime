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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_INTERFACE_H

#include <ipc_types.h>
#include <iremote_broker.h>
#include <list>
#include <vector>

#include "ability_connect_callback_interface.h"
#include "ability_manager_ipc_interface_code.h"
#include "ability_running_info.h"
#include "ability_scheduler_interface.h"
#include "ability_start_setting.h"
#include "ability_state.h"
#include "ability_state_data.h"
#include "app_debug_listener_interface.h"
#include "auto_startup_info.h"
#include "dms_continueInfo.h"
#include "exit_reason.h"
#include "extension_running_info.h"
#include "free_install_observer_interface.h"
#include "iability_controller.h"
#include "iability_manager_collaborator.h"
#include "iacquire_share_data_callback_interface.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "iprepare_terminate_callback_interface.h"
#include "mission_info.h"
#include "mission_listener_interface.h"
#include "mission_snapshot.h"
#include "remote_mission_listener_interface.h"
#include "remote_on_listener_interface.h"
#include "running_process_info.h"
#include "sender_info.h"
#include "snapshot.h"
#include "start_options.h"
#include "user_callback.h"
#include "system_memory_attr.h"
#include "ui_extension_ability_connect_info.h"
#include "ui_extension_host_info.h"
#include "ui_extension_session_info.h"
#include "ui_extension_window_command.h"
#include "uri.h"
#include "want.h"
#include "want_receiver_interface.h"
#include "want_sender_info.h"
#include "want_sender_interface.h"
#include "dialog_session_info.h"
#include "window_config.h"
#ifdef SUPPORT_SCREEN
#include "window_manager_service_handler.h"
#include "ability_first_frame_state_observer_interface.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
class IStatusBarDelegate;
}

namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;
using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;
using UIExtensionHostInfo = AbilityRuntime::UIExtensionHostInfo;
using UIExtensionSessionInfo = AbilityRuntime::UIExtensionSessionInfo;
#ifdef SUPPORT_SCREEN
using IAbilityFirstFrameStateObserver = AppExecFwk::IAbilityFirstFrameStateObserver;
#endif

constexpr const char* ABILITY_MANAGER_SERVICE_NAME = "AbilityManagerService";
const int DEFAULT_INVAL_VALUE = -1;
const int DELAY_LOCAL_FREE_INSTALL_TIMEOUT = 40000;
const int DELAY_REMOTE_FREE_INSTALL_TIMEOUT = 30000 + DELAY_LOCAL_FREE_INSTALL_TIMEOUT;
constexpr const char* FROM_REMOTE_KEY = "freeInstallFromRemote";
/**
 * @class IAbilityManager
 * IAbilityManager interface is used to access ability manager services.
 */
class IAbilityManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.AbilityManager")

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * StartAbilityWithSpecifyTokenId with want and specialId, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param specialId the caller Id.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityWithSpecifyTokenId(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint32_t specifyTokenId,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * StartAbility by insight intent, send want to ability manager service.
     *
     * @param want Ability want.
     * @param callerToken caller ability token.
     * @param intentId insight intent id.
     * @param userId userId of target ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartAbilityByInsightIntent(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint64_t intentId,
        int32_t userId = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param requestCode the resultCode of the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param callerToken caller ability token.
     * @param asCallerSourceToken source caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityAsCaller(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken caller ability token.
     * @param asCallerSourceToken source caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityAsCaller(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * Starts a new ability for result using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param callerToken current caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @param userId Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityForResultAsCaller(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * Starts a new ability for result using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken current caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @param userId Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityForResultAsCaller(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * Start ui session ability with extension session info, send session info to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param sessionInfo the information of UIExtensionContentSession.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByUIContentSession(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

/**
     * Start ui session ability with extension session info, send session info to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken, caller ability token.
     * @param sessionInfo the information of UIExtensionContentSession.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByUIContentSession(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * Start extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED)
    {
        return 0;
    }

  /**
     * Create UIExtension with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RequestModalUIExtension(const Want &want)
    {
        return 0;
    }

    /**
     * Preload UIExtension with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param hostBundleName, the caller application bundle name.
     * @param userId, the extension runs in.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PreloadUIExtensionAbility(const Want &want, std::string &hostBundleName,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
    {
        return 0;
    }

        virtual int ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
    {
        return 0;
    }

    /**
     * Start ui extension ability with extension session info, send extension session info to ability manager service.
     *
     * @param extensionSessionInfo the extension session info of the ability to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUIExtensionAbility(
        const sptr<SessionInfo> &extensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * Start ui ability with want, send want to ability manager service.
     *
     * @param sessionInfo the session info of the ability to start.
     * @param isColdStart the session info of the ability is or not cold start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo, bool &isColdStart, uint32_t sceneFlag = 0)
    {
        return 0;
    }

    /**
     * Stop extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to stop.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be stopped.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopExtensionAbility(
        const Want& want,
        const sptr<IRemoteObject>& callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED)
    {
        return 0;
    }

    virtual int GetAppMemorySize()
    {
        return 0;
    }

    virtual bool IsRamConstrainedDevice()
    {
        return false;
    }

    virtual AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId = true)
    {
        return {};
    }

    virtual AppExecFwk::ElementName GetElementNameByToken(sptr<IRemoteObject> token,
        bool isNeedLocalDeviceId = true)
    {
        return {};
    }

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) = 0;
    
    /**
     * BackToCallerAbilityWithResult, return to the caller ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @param callerRequestCode, the requestCode of caller ability.·
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BackToCallerAbilityWithResult(const sptr<IRemoteObject> &token, int resultCode,
        const Want *resultWant, int64_t callerRequestCode)
    {
        return 0;
    };

    /**
     * TerminateUIExtensionAbility, terminate the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to terminate.
     * @param resultCode, the resultCode of the ui extension ability to terminate.
     * @param resultWant, the Want of the ui extension ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        int resultCode, const Want *resultWant = nullptr)
    {
        return 0;
    }

    /**
     *  CloseUIAbilityBySCB, close the special ability by scb.
     *
     * @param sessionInfo the session info of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
    {
        return 0;
    }

    /**
     * SendResultToAbility, send the result to ability.
     *
     * @param requestCode, the requestCode of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendResultToAbility(int requestCode, int resultCode, Want &resultWant)
    {
        return 0;
    }

    /**
     * MoveAbilityToBackground.
     *
     * @param token, the token of the ability to move.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveAbilityToBackground(const sptr<IRemoteObject> &token)
    {
        return 0;
    };

    /**
     * Move the UIAbility to background, called by app self.
     *
     * @param token the token of the ability to move.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t MoveUIAbilityToBackground(const sptr<IRemoteObject> token)
    {
        return 0;
    };

    /**
     * CloseAbility, close the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) = 0;

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, the token of the ability to minimize.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false) = 0;

    /**
     * MinimizeUIExtensionAbility, minimize the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to minimize.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        bool fromUser = false)
    {
        return 0;
    };

    /**
     * MinimizeUIAbilityBySCB, minimize the special ui ability by scb.
     *
     * @param sessionInfo the session info of the ability to minimize.
     * @param fromUser, Whether form user.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool fromUser = false,
        uint32_t sceneFlag = 0)
    {
        return 0;
    };

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ConnectAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Connect ability common method.
     *
     * @param want, special want for service type's ability.
     * @param connect, callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param extensionType, type of the extension.
     * @param userId, the service user ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ConnectAbilityCommon(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isQueryExtensionOnly = false)
    {
        return 0;
    }

    /**
     * Connect ui extension ability.
     *
     * @param want, special want for the ui extension ability.
     * @param connect, callback used to notify caller the result of connecting or disconnecting.
     * @param sessionInfo the extension session info of the ability to connect.
     * @param userId, the extension runs in.
     * @param connectInfo the connect info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<SessionInfo> &sessionInfo, int32_t userId = DEFAULT_INVAL_VALUE,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr)
    {
        return 0;
    }

    /**
     * DisconnectAbility, disconnect session with service ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbility(sptr<IAbilityConnection> connect) = 0;

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param authority, a string to identify a data ability, decoded from uri.
     * @param tryBind, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityScheduler, specifies the data ability that will be released.
     * @param callerToken, specifies the caller ability token.
     * @return returns ERR_OK if succeeded, or error codes for failed.
     */
    virtual int ReleaseDataAbility(
        sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * AttachAbilityThread, ability call this interface after loaded.
     *
     * @param scheduler,.the interface handler of kit ability.
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) = 0;

    /**
     * AbilityTransitionDone, ability call this interface after life cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability life cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) = 0;

    /**
     * AbilityWindowConfigTransitionDone, ability call this interface after life cycle was changed.
     *
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AbilityWindowConfigTransitionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig)
        {
            return 0;
        }

    /**
     * ScheduleConnectAbilityDone, service ability call this interface while session was connected.
     *
     * @param token,.service ability's token.
     * @param remoteObject,.the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleConnectAbilityDone(
        const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) = 0;

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) = 0;

    /**
     * ScheduleCommandAbilityDone, service ability call this interface while session was commanded.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) = 0;

    virtual int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<AAFwk::SessionInfo> &sessionInfo,
        AAFwk::WindowCommand winCmd,
        AAFwk::AbilityCommand abilityCmd) = 0;

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual void DumpState(const std::string &args, std::vector<std::string> &state) = 0;
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID) = 0;

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @param token ability's token.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE,
        const sptr<IRemoteObject> &token = nullptr) = 0;

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillProcess(const std::string &bundleName, const bool clearPageStack = true) = 0;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * force timeout ability.
     *
     * @param abilityName.
     * @param state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) = 0;
    #endif

    /**
     * Uninstall app
     *
     * @param bundleName bundle name of uninstalling app.
     * @param uid uid of bundle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UninstallApp(const std::string &bundleName, int32_t uid)
    {
        return 0;
    }

    /**
     * Uninstall app
     *
     * @param bundleName bundle name of uninstalling app.
     * @param uid uid of bundle.
     * @param appIndex the app index of app clone.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UninstallApp(const std::string &bundleName, int32_t uid, int32_t appIndex)
    {
        return 0;
    }

    /**
     * Upgrade app, record exit reason and kill application
     *
     * @param bundleName bundle name of upgrading app.
     * @param uid uid of bundle.
     * @param exitMsg the exit reason message.
     * @param appIndex the app index of app clone.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpgradeApp(const std::string &bundleName, const int32_t uid, const std::string &exitMsg,
        int32_t appIndex = 0)
    {
        return 0;
    }

    virtual sptr<IWantSender> GetWantSender(
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken) = 0;

    virtual int SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo) = 0;

    virtual void CancelWantSender(const sptr<IWantSender> &sender) = 0;

    virtual int GetPendingWantUid(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantUserId(const sptr<IWantSender> &target) = 0;

    virtual std::string GetPendingWantBundleName(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantCode(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantType(const sptr<IWantSender> &target) = 0;

    virtual void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) = 0;

    virtual void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) = 0;

    virtual int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) = 0;

    virtual int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) = 0;

    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) = 0;

    virtual int ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo, const sptr<IRemoteObject> &callback)
    {
        return 0;
    }

    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) = 0;

    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) = 0;

    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) = 0;

    virtual int NotifyContinuationResult(int32_t missionId, int32_t result) = 0;

    virtual int LockMissionForCleanup(int32_t missionId) = 0;

    virtual int UnlockMissionForCleanup(int32_t missionId) = 0;

    virtual void SetLockedState(int32_t sessionId, bool lockedState)
    {
        return;
    }

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) = 0;

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) = 0;

    virtual int GetMissionInfos(
        const std::string &deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos) = 0;

    virtual int GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo) = 0;

    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) = 0;

    virtual int CleanMission(int32_t missionId) = 0;

    virtual int CleanAllMissions() = 0;

    virtual int MoveMissionToFront(int32_t missionId) = 0;

    virtual int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) = 0;

    virtual int MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
    {
        return 0;
    }

    virtual int MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result)
    {
        return 0;
    }

    virtual int RegisterSessionHandler(const sptr<IRemoteObject> &object)
    {
        return 0;
    }

    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param accountId Indicates the account to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId = DEFAULT_INVAL_VALUE) = 0;

    /**
     * CallRequestDone, after invoke callRequest, ability will call this interface to return callee.
     *
     * @param token, ability's token.
     * @param callStub, ability's callee.
     */
    virtual void CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub) {};

    /**
     * Release the call between Ability, disconnect session with common ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ReleaseCall(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element) = 0;

    virtual int StartUser(int userId, sptr<IUserCallback> callback) = 0;

    virtual int StopUser(int userId, const sptr<IUserCallback> &callback) = 0;

    virtual int LogoutUser(int32_t userId)
    {
        return 0;
    }

    virtual int SetMissionContinueState(const sptr<IRemoteObject> &token, const AAFwk::ContinueState &state)
    {
        return 0;
    };

#ifdef SUPPORT_SCREEN
    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) = 0;

    virtual int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) = 0;

    /**
     * Called to update mission snapshot.
     * @param token The target ability.
     * @param pixelMap The snapshot.
     */
    virtual void UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &pixelMap) {};

    /**
     * Register the WindowManagerService handler
     *
     * @param handler Indidate handler of WindowManagerService.
     * @return ErrCode Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
        bool animationEnabled)
    {
        return 0;
    }

    /**
     * WindowManager notification AbilityManager after the first frame is drawn.
     *
     * @param abilityToken Indidate token of ability.
     */
    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) = 0;

    /**
     * WindowManager notification AbilityManager after the first frame is drawn.
     *
     * @param sessionId Indidate session id.
     */
    virtual void CompleteFirstFrameDrawing(int32_t sessionId)
    {}

    /**
     * PrepareTerminateAbility, prepare terminate the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param callback callback.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PrepareTerminateAbility(const sptr<IRemoteObject> &token, sptr<IPrepareTerminateCallback> &callback)
    {
        return 0;
    }

    virtual int GetDialogSessionInfo(const std::string &dialogSessionId, sptr<DialogSessionInfo> &dialogSessionInfo)
    {
        return 0;
    }

    virtual int SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllow)
    {
        return 0;
    }

    /**
     * Register ability first frame state observer.
     * @param observer Is ability first frame state observer.
     * @param bundleName Is bundleName of the app to observe.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer,
        const std::string &targetBundleName)
    {
        return 0;
    }

    /**
     * Unregister ability first frame state observer.
     * @param observer Is ability first frame state observer.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer)
    {
        return 0;
    }

#endif

    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) = 0;

    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) = 0;

    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) = 0;

    /**
     * Start synchronizing remote device mission
     * @param devId, deviceId.
     * @param fixConflict, resolve synchronizing conflicts flag.
     * @param tag, call tag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag) = 0;

    /**
     * Stop synchronizing remote device mission
     * @param devId, deviceId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopSyncRemoteMissions(const std::string &devId) = 0;

    virtual int RegisterMissionListener(const std::string &deviceId, const sptr<IRemoteMissionListener> &listener) = 0;

    virtual int RegisterOnListener(const std::string &type, const sptr<IRemoteOnListener> &listener)
    {
        return 0;
    }

    virtual int RegisterOffListener(const std::string &type, const sptr<IRemoteOnListener> &listener)
    {
        return 0;
    }

    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) = 0;

    /**
     * Set ability controller.
     *
     * @param abilityController, The ability controller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) = 0;

    /**
     * Is user a stability test.
     *
     * @return Returns true if user is a stability test.
     */
    virtual bool IsRunningInStabilityTest() = 0;

    /**
     * @brief Register the snapshot handler
     * @param handler snapshot handler
     * @return int Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) = 0;

    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) = 0;

    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) = 0;

    /**
     * GetTopAbility, get the token of top ability.
     *
     * @param token, the token of top ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetTopAbility(sptr<IRemoteObject> &token) = 0;

    virtual int CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused)
    {
        return 0;
    }

    /**
     * The delegator calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) = 0;

    /**
     * The delegator calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) = 0;

    /**
     * Calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) = 0;

    /**
     * Calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) = 0;

    /**
     * Get mission id by ability token.
     *
     * @param token The token of ability.
     * @return Returns -1 if do not find mission, otherwise return mission id.
     */
    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) = 0;

    /**
     * Get ability token by connect.
     *
     * @param token The token of ability.
     * @param callStub The callee object.
     */
    virtual void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) = 0;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAmsService() = 0;

    /**
     * Block ability.
     *
     * @param abilityRecordId The Ability Record Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAbility(int32_t abilityRecordId) = 0;

    /**
     * Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService() = 0;
    #endif

    /**
     * Called when client complete dump.
     *
     * @param infos The dump info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    /**
     * Call free install from remote.
     *
     * @param want, the want of the ability to start.
     * @param callback, Callback from remote.
     * @param userId, Designation User ID.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * Add free install observer.
     *
     * @param callerToken, The caller ability token.
     * @param observer, The observer of the ability to free install start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AddFreeInstallObserver(const sptr<IRemoteObject> &callerToken,
        const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
    {
        return 0;
    }

    virtual void EnableRecoverAbility(const sptr<IRemoteObject>& token) {};
    virtual void SubmitSaveRecoveryInfo(const sptr<IRemoteObject>& token) {};
    virtual void ScheduleRecoverAbility(const sptr<IRemoteObject> &token, int32_t reason,
        const Want *want = nullptr) {};

    virtual void ScheduleClearRecoveryPageStack() {};

    /**
     * Called to verify that the MissionId is valid.
     * @param missionIds Query mission list.
     * @param results Output parameters, return results up to 20 query results.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t IsValidMissionIds(
        const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results) = 0;

    /**
     * Query whether the application of the specified PID and UID has been granted a certain permission
     * @param permission
     * @param pid Process id
     * @param uid
     * @return Returns ERR_OK if the current process has the permission, others on failure.
     */
    virtual int VerifyPermission(const std::string &permission, int pid, int uid)
    {
        return 0;
    }

    /**
     * Request dialog service with want, send want to ability manager service.
     *
     * @param want, the want of the dialog service to start.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    /**
     * Report drawn completed.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * Acquire the shared data.
     * @param missionId The missionId of Target ability.
     * @param shareData The IAcquireShareData object.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t AcquireShareData(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData)
    {
        return 0;
    }

    /**
     * Notify sharing data finished.
     * @param token The token of ability.
     * @param resultCode The result of sharing data.
     * @param uniqueId The uniqueId from request object.
     * @param wantParam The params of acquiring sharing data from target ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ShareDataDone(const sptr<IRemoteObject>& token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam)
    {
        return 0;
    }

    /**
     * Force app exit and record exit reason.
     * @param pid Process id .
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ForceExitApp(const int32_t pid, const ExitReason &exitReason)
    {
        return 0;
    }

    /**
     * Record app exit reason.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RecordAppExitReason(const ExitReason &exitReason)
    {
        return 0;
    }

    /**
     * Record the process exit reason before the process being killed.
     * @param pid The process id.
     * @param exitReason The reason of process exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason)
    {
        return 0;
    }

    /**
     * Set rootSceneSession by SCB.
     *
     * @param rootSceneSession Indicates root scene session of SCB.
     */
    virtual void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession) {}

    /**
     * Call UIAbility by SCB.
     *
     * @param sessionInfo the session info of the ability to be called.
     * @param isColdStart the session of the ability is or not cold start.
     */
    virtual void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isColdStart) {}

    /**
     * Start specified ability by SCB.
     *
     * @param want Want information.
     */
    virtual void StartSpecifiedAbilityBySCB(const Want &want) {};

    /**
     * Notify sandbox app the result of saving file.
     * @param want Result of saving file, which contains the file's uri if success.
     * @param resultCode Indicates the action's result.
     * @param requestCode Pass the requestCode to match request.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifySaveAsResult(const Want &want, int resultCode, int requestCode)
    {
        return 0;
    }

    /**
     * Set sessionManagerService
     * @param sessionManagerService the point of sessionManagerService.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetSessionManagerService(const sptr<IRemoteObject> &sessionManagerService)
    {
        return 0;
    }

    /**
     * @brief Register collaborator.
     * @param type collaborator type.
     * @param impl collaborator.
     * @return 0 or else.
    */
    virtual int32_t RegisterIAbilityManagerCollaborator(int32_t type, const sptr<IAbilityManagerCollaborator> &impl)
    {
        return 0;
    }

    /**
     * @brief Unregister collaborator.
     * @param type collaborator type.
     * @return 0 or else.
    */
    virtual int32_t UnregisterIAbilityManagerCollaborator(int32_t type)
    {
        return 0;
    }

    virtual int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate)
    {
        return 0;
    }

    virtual int32_t KillProcessWithPrepareTerminate(const std::vector<int32_t>& pids)
    {
        return 0;
    }

    /**
     * @brief Register auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
    {
        return 0;
    }

    /**
     * @brief Unregister auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
    {
        return 0;
    }

    /**
     * @brief Set every application auto start up state.
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetApplicationAutoStartup(const AutoStartupInfo &info)
    {
        return 0;
    }

    /**
     * @brief Cancel every application auto start up .
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CancelApplicationAutoStartup(const AutoStartupInfo &info)
    {
        return 0;
    }

    /**
     * @brief Query auto startup state all application.
     * @param infoList Output parameters, return auto startup info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
    {
        return 0;
    }

    /**
     * PrepareTerminateAbilityBySCB, prepare to terminate ability by scb.
     *
     * @param sessionInfo the session info of the ability to start.
     * @param isPrepareTerminate the result of ability onPrepareToTerminate
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PrepareTerminateAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isPrepareTerminate)
    {
        return 0;
    }

    /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) = 0;

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) = 0;

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
     * @brief Execute intent.
     * @param key The key of intent executing client.
     * @param callerToken Caller ability token.
     * @param param The Intent execute param.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ExecuteIntent(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param) = 0;

    /**
     * @brief Check if ability controller can start.
     * @param want The want of ability to start.
     * @return Return true to allow ability to start, or false to reject.
     */
    virtual bool IsAbilityControllerStart(const Want &want)
    {
        return true;
    }

    /**
     * @brief Called when insight intent execute finished.
     *
     * @param token ability's token.
     * @param intentId insight intent id.
     * @param result insight intent execute result.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result) = 0;

    /**
     * @brief Set application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) = 0;

    /**
     * @brief Cancel application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) = 0;

    /**
     * @brief Get foreground ui abilities.
     * @param list Foreground ui abilities.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list) = 0;

    /**
     * @brief Open file by uri.
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @return int The file descriptor.
     */
    virtual int32_t OpenFile(const Uri& uri, uint32_t flag)
    {
        return 0;
    }

    /**
     * @brief Update session info.
     * @param sessionInfos The vector of session info.
     */
    virtual int32_t UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, int32_t userId,
        std::vector<int32_t> &sessionIds)
    {
        return 0;
    }

    /**
     * @brief Restart app self.
     * @param want The ability type must be UIAbility.
     * @param isAppRecovery True indicates that the app is restarted because of recovery.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RestartApp(const AAFwk::Want &want, bool isAppRecovery = false)
    {
        return 0;
    }

    /**
     * @brief Get host info of root caller.
     *
     * @param token The ability token.
     * @param hostInfo The host info of root caller.
     * @param userId The user id.
     * @return int32_t Returns 0 on success, others on failure.
     */
    virtual int32_t GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token, UIExtensionHostInfo &hostInfo,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * @brief Get ui extension session info
     *
     * @param token The ability token.
     * @param uiExtensionSessionInfo The ui extension session info.
     * @param userId The user id.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token,
        UIExtensionSessionInfo &uiExtensionSessionInfo, int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * Open link of ability and atomic service.
     *
     * @param want Ability want.
     * @param callerToken Caller ability token.
     * @param userId User ID.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
    */
    virtual int32_t OpenLink(const Want& want, sptr<IRemoteObject> callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * @brief Pop-up launch of full-screen atomic service.
     * @param want The want with parameters.
     * @param callerToken caller ability token.
     * @param requestCode Ability request code.
     * @param userId The User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t OpenAtomicService(Want& want, const StartOptions &options, sptr<IRemoteObject> callerToken,
        int32_t requestCode = DEFAULT_INVAL_VALUE, int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /*
     * Set the enable status for starting and stopping resident processes.
     * The caller application can only set the resident status of the configured process.
     * @param bundleName The bundle name of the resident process.
     * @param enable Set resident process enable status.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetResidentProcessEnabled(const std::string &bundleName, bool enable)
    {
        return 0;
    }

    /**
     * @brief Querying whether to allow embedded startup of atomic service.
     *
     * @param token The caller UIAbility token.
     * @param appId The ID of the application to which this bundle belongs.
     * @return Returns true to allow ability to start, or false to reject.
     */
    virtual bool IsEmbeddedOpenAllowed(sptr<IRemoteObject> callerToken, const std::string &appId)
    {
        return true;
    }

    /**
     * @brief Request to display assert fault dialog.
     * @param callback Listen for user operation callbacks.
     * @param wantParams Assert dialog box display information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RequestAssertFaultDialog(const sptr<IRemoteObject> &callback, const AAFwk::WantParams &wantParams)
    {
        return -1;
    }

    /**
     * @brief Notify the operation status of the user.
     * @param assertFaultSessionId Indicates the request ID of AssertFault.
     * @param userStatus Operation status of the user.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyDebugAssertResult(uint64_t assertFaultSessionId, AAFwk::UserStatus userStatus)
    {
        return -1;
    }

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartShortcut(const Want &want, const StartOptions &startOptions)
    {
        return 0;
    }

    /**
     * Get ability state by persistent id.
     *
     * @param persistentId, the persistentId of the session.
     * @param state Indicates the ability state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAbilityStateByPersistentId(int32_t persistentId, bool &state)
    {
        return 0;
    }

    /**
     * Transfer resultCode & want to ability manager service.
     *
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t TransferAbilityResultForExtension(const sptr<IRemoteObject> &callerToken, int32_t resultCode,
        const Want &want)
    {
        return 0;
    }

    /**
     * Notify ability manager service frozen process.
     *
     * @param pidList, the pid list of the frozen process.
     * @param uid, the uid of the frozen process.
     */
    virtual void NotifyFrozenProcessByRSS(const std::vector<int32_t> &pidList, int32_t uid)
    {
        return;
    }

    /**
     *  Request to clean UIAbility from user.
     *
     * @param sessionInfo the session info of the ability to clean.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CleanUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
    {
        return 0;
    }

    /**
     * Open atomic service window prior to finishing free install.
     *
     * @param bundleName, the bundle name of the atomic service.
     * @param moduleName, the module name of the atomic service.
     * @param abilityName, the ability name of the atomic service.
     * @param startTime, the starting time of the free install task.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t PreStartMission(const std::string& bundleName, const std::string& moduleName,
        const std::string& abilityName, const std::string& startTime)
    {
        return 0;
    }

    /**
     * Terminate the mission.
     *
     * @param missionId, The mission id of the UIAbility need to be terminated.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t TerminateMission(int32_t missionId)
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_INTERFACE_H
