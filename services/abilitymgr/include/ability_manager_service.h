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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H

#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <shared_mutex>
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>

#include "ability_auto_startup_service.h"
#include "ability_bundle_event_callback.h"
#include "ability_config.h"
#include "ability_connect_manager.h"
#include "ability_debug_deal.h"
#include "ability_event_handler.h"
#include "ability_info.h"
#include "ability_manager_event_subscriber.h"
#include "ability_manager_stub.h"
#include "ams_configuration_parameter.h"
#include "app_debug_listener_interface.h"
#include "app_exit_reason_helper.h"
#include "app_mgr_interface.h"
#include "app_scheduler.h"
#include "auto_startup_info.h"
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_observer.h"
#endif
#include "bundle_constants.h"
#include "bundle_mgr_helper.h"
#include "data_ability_manager.h"
#include "deeplink_reserve/deeplink_reserve_config.h"
#include "event_report.h"
#include "free_install_manager.h"
#include "iacquire_share_data_callback_interface.h"
#include "interceptor/ability_interceptor_executer.h"
#include "iremote_object.h"
#include "mission_list_manager_interface.h"
#include "parameter.h"
#include "pending_want_manager.h"
#include "permission_verification.h"
#include "resident_process_manager.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#include "start_ability_handler.h"
#include "sub_managers_helper.h"
#include "system_ability.h"
#include "task_handler_wrap.h"
#include "uri.h"
#include "user_controller.h"
#ifdef SUPPORT_GRAPHICS
#include "implicit_start_processor.h"
#include "system_dialog_scheduler.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
class IStatusBarDelegate;
}
namespace Rosen {
class FocusChangeInfo;
}

namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr int32_t U0_USER_ID = 0;
constexpr int32_t INVALID_USER_ID = -1;
constexpr const char* KEY_SESSION_ID = "com.ohohs.param.sessionId";
using OHOS::AppExecFwk::IAbilityController;
class PendingWantManager;
struct StartAbilityInfo;
class WindowFocusChangedListener;

/**
 * @class AbilityManagerService
 * AbilityManagerService provides a facility for managing ability life cycle.
 */
class AbilityManagerService : public SystemAbility,
                              public AbilityManagerStub,
                              public AppStateCallback,
                              public std::enable_shared_from_this<AbilityManagerService> {
    DECLARE_DELAYED_SINGLETON(AbilityManagerService)
    DECLEAR_SYSTEM_ABILITY(AbilityManagerService)
public:
    static std::shared_ptr<AbilityManagerService> GetPubInstance();

    void OnStart() override;
    void OnStop() override;

    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    ServiceRunningState QueryServiceState() const;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param requestCode, Ability request code.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want, int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * StartAbilityWithSpecifyTokenIdInner with want and specialId, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param specialId the caller Id.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbilityWithSpecifyTokenIdInner(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint32_t specifyTokenId,
        bool isPendingWantCaller,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * Starts a new ability with specific start options and specialId, send want to ability manager service.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @param specifyTokenId The Caller ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbilityWithSpecifyTokenIdInner(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        bool isPendingWantCaller,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        uint32_t specifyTokenId = 0);

    /**
     * StartAbilityWithSpecifyTokenId with want and specialId, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param specialId the caller Id.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityWithSpecifyTokenId(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint32_t specifyTokenId,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * StartAbility by insight intent, send want to ability manager service.
     *
     * @param want Ability want.
     * @param callerToken caller ability token.
     * @param intentId insight intent id.
     * @param userId userId of target ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartAbilityByInsightIntent(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint64_t intentId,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability with specific start options.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param callerToken current caller ability token.
     * @param asCallerSourceToken source caller ability token
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityAsCaller(
            const Want &want,
            const sptr<IRemoteObject> &callerToken,
            sptr<IRemoteObject> asCallerSourceToken,
            int32_t userId = DEFAULT_INVAL_VALUE,
            int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken current caller ability token.
     * @param asCallerSourceToken source caller ability token
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
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability for result using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param callerToken current caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @param userId Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbilityForResultAsCaller(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

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
    int StartAbilityForResultAsCaller(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Start ui session ability with extension session info, send session info to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken caller ability token.
     * @param sessionInfo the information of UIExtensionContentSession.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByUIContentSession(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId,
        int requestCode) override;

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
        int32_t userId,
        int requestCode) override;

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
        int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Pop-up launch of full-screen atomic service.
     *
     * @param want The want with parameters.
     * @param callerToken caller ability token.
     * @param requestCode Ability request code.
     * @param userId The User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t OpenAtomicService(Want& want, const StartOptions &options, sptr<IRemoteObject> callerToken,
        int32_t requestCode = DEFAULT_INVAL_VALUE, int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * @brief Querying whether to allow embedded startup of atomic service.
     *
     * @param token The caller UIAbility token.
     * @param appId The ID of the application to which this bundle belongs.
     * @return Returns true to allow ability to start, or false to reject.
     */
    virtual bool IsEmbeddedOpenAllowed(sptr<IRemoteObject> callerToken, const std::string &appId) override;

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
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED) override;

    /**
     * Requset modal UIExtension with want, send want to ability manager service.
     *
     * @param want, the want contains ability info about caller and called.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RequestModalUIExtension(const Want &want) override;

    int PreloadUIExtensionAbility(const Want &want, std::string &hostBundleName,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, std::string &bundleName);

    int ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow) override;

    int ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow) override;

    /**
     * Start ui extension ability with extension session info, send extension session info to ability manager service.
     *
     * @param extensionSessionInfo the extension session info of the ability to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUIExtensionAbility(
        const sptr<SessionInfo> &extensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Start ui ability with want, send want to ability manager service.
     *
     * @param sessionInfo the session info of the ability to start.
     * @param isColdStart the session info of the ability is or not cold start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo, bool &isColdStart, uint32_t sceneFlag = 0) override;

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
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED) override;

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override;
    
    /**
     * BackToCallerAbilityWithResult, return to the caller ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @param callerRequestCode, the requestCode of caller ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BackToCallerAbilityWithResult(const sptr<IRemoteObject> &token, int resultCode,
        const Want *resultWant, int64_t callerRequestCode) override;

    /**
     * TerminateAbility, terminate the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to terminate.
     * @param resultCode, the resultCode of the ui extension ability to terminate.
     * @param resultWant, the Want of the ui extension ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        int resultCode = DEFAULT_INVAL_VALUE, const Want *resultWant = nullptr) override;

    /**
     *  CloseUIAbilityBySCB, close the special ability by scb.
     *
     * @param sessionInfo the session info of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo) override;

    /**
     * SendResultToAbility with want, return want from ability manager service.
     *
     * @param requestCode, request code.
     * @param resultCode, resultCode to return.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendResultToAbility(int32_t requestCode, int32_t resultCode, Want& resultWant) override;

    /**
     * MoveAbilityToBackground.
     *
     * @param token, the token of the ability to move background.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveAbilityToBackground(const sptr<IRemoteObject> &token) override;

    /**
     * Move the UIAbility to background, called by app self.
     *
     * @param token the token of the ability to move.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t MoveUIAbilityToBackground(const sptr<IRemoteObject> token) override;

    /**
     * CloseAbility, close the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override;

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, ability token.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false) override;

    /**
     * MinimizeUIExtensionAbility, minimize the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to minimize.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        bool fromUser = false) override;

    /**
     * MinimizeUIAbilityBySCB, minimize the special ability by scb.
     *
     * @param sessionInfo the extension session info of the ability to minimize.
     * @param fromUser, Whether form user.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool fromUser = false,
        uint32_t sceneFlag = 0) override;

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
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    virtual int ConnectAbilityCommon(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isQueryExtensionOnly = false) override;

    virtual int ConnectUIExtensionAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr) override;

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
    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override;

    /**
     * ContinueMission, continue ability from mission center.
     *
     * @param srcDeviceId, origin deviceId.
     * @param dstDeviceId, target deviceId.
     * @param bundleName, indicates which ability to continue.
     * @param callBack, notify result back.
     * @param wantParams, extended params.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo,
        const sptr<IRemoteObject> &callback) override;

    /**
     * ContinueAbility, continue ability to ability.
     *
     * @param deviceId, target deviceId.
     * @param missionId, indicates which ability to continue.
     * @param versionCode, target bundle version.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) override;

    /**
     * StartContinuation, continue ability to remote.
     *
     * @param want, Indicates the ability to start.
     * @param abilityToken, Caller ability token.
     * @param status, continue status.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override;

    /**
     * NotifyCompleteContinuation, notify continuation complete to dms.
     * @param deviceId, source device which start a continuation.
     * @param sessionId, represent a continuaion.
     * @param isSuccess, continuation result.
     * @return
     */
    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override;

    /**
     * NotifyContinuationResult, notify continue result to ability.
     *
     * @param missionId, Caller mission id.
     * @param result, continuation result.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyContinuationResult(int32_t missionId, int32_t result) override;

    /**
     * RegisterMissionListener, register remote device mission listener.
     *
     * @param deviceId, Indicates the remote device Id.
     * @param listener, listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;

    /**
     * RegisterOnListener, register on notify mission listener.
     *
     * @param type, Indicates the notify type.
     * @param listener, listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterOnListener(const std::string &type,
        const sptr<IRemoteOnListener> &listener) override;

    /**
     * RegisterOffListener, register on notify mission listener.
     *
     * @param type, Indicates the notify type.
     * @param listener, listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterOffListener(const std::string &type,
        const sptr<IRemoteOnListener> &listener) override;

    /**
     * UnRegisterMissionListener, unregister remote device mission listener.
     *
     * @param deviceId, Indicates the remote device Id.
     * @param listener, listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener)override;

    virtual int DisconnectAbility(sptr<IAbilityConnection> connect) override;

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param tryBind, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) override;

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityToken, specifies the data ability that will be released.
     * @param callerToken, specifies the caller ability token.
     * @return returns ERR_OK if succeeded, or error codes for failed.
     */
    virtual int ReleaseDataAbility(
        sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken) override;

    /**
     * AttachAbilityThread, ability call this interface after loaded.
     *
     * @param scheduler,.the interface handler of kit ability.
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AttachAbilityThread(
        const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) override;

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) override;

    /**
     * AbilityWindowConfigTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AbilityWindowConfigTransitionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig) override;

    /**
     * ScheduleConnectAbilityDone, service ability call this interface while session was connected.
     *
     * @param token,.service ability's token.
     * @param remoteObject,.the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleConnectAbilityDone(
        const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) override;

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) override;

    /**
     * ScheduleCommandAbilityDone, service ability call this interface while session was commanded.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) override;

    virtual int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd) override;

    /**
     *  Request to clean UIAbility from user.
     *
     * @param sessionInfo the session info of the ability to clean.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CleanUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo) override;

    std::shared_ptr<TaskHandlerWrap> GetTaskHandler() const
    {
        return taskHandler_;
    }

    /**
     * GetEventHandler, get the ability manager service's handler.
     *
     * @return Returns AbilityEventHandler ptr.
     */
    std::shared_ptr<AbilityEventHandler> GetEventHandler();

    /**
     * @brief Ability hidump.
     * @param fd Indicates the fd.
     * @param args Indicates the params.
     * @return Returns the dump result.
     */
    int Dump(int fd, const std::vector<std::u16string>& args) override;

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual void DumpState(const std::string &args, std::vector<std::string> &info) override;
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID) override;

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @param token ability's token.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE,
        const sptr<IRemoteObject> &token = nullptr) override;

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillProcess(const std::string &bundleName, const bool clearPageStack = true) override;

    /**
     * Uninstall app
     *
     * @param bundleName bundle name of uninstalling app.
     * @param uid uid of bundle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UninstallApp(const std::string &bundleName, int32_t uid) override;

    /**
     * Uninstall app
     *
     * @param bundleName bundle name of uninstalling app.
     * @param uid uid of bundle.
     * @param appIndex the app index of app clone.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UninstallApp(const std::string &bundleName, int32_t uid, int32_t appIndex) override;

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
        int32_t appIndex = 0) override;

    virtual sptr<IWantSender> GetWantSender(
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken) override;

    virtual int SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo) override;

    virtual void CancelWantSender(const sptr<IWantSender> &sender) override;

    virtual int GetPendingWantUid(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantUserId(const sptr<IWantSender> &target) override;

    virtual std::string GetPendingWantBundleName(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantCode(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantType(const sptr<IWantSender> &target) override;

    virtual void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override;

    virtual void UnregisterCancelListener(
        const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override;

    virtual int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) override;

    virtual int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) override;

    virtual int RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer) override;

    virtual int UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer) override;

#ifdef WITH_DLP
    virtual int GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos) override;
#endif // WITH_DLP

    virtual int GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData) override;

    virtual int LockMissionForCleanup(int32_t missionId) override;

    virtual int UnlockMissionForCleanup(int32_t missionId) override;

    virtual void SetLockedState(int32_t sessionId, bool lockedState) override;

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override;

    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo) override;

    virtual int CleanMission(int32_t missionId) override;

    virtual int CleanAllMissions() override;

    virtual int MoveMissionToFront(int32_t missionId) override;

    virtual int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) override;

    virtual int MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override;

    virtual int MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
        std::vector<int32_t>& result) override;

    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) override;

    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) override;

    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;

    virtual int StopSyncRemoteMissions(const std::string& devId) override;

    virtual int GetAppMemorySize() override;

    virtual bool IsRamConstrainedDevice() override;
    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param accountId Indicates the account to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId = DEFAULT_INVAL_VALUE) override;

    /**
     * As abilityRequest is prepared, just execute starting ability procedure.
     * By now, this is only used by start_ability_sandbox_savefile.
     * @param abilityRequest, Prepared with all info for starting a ability.
     * @param validUserId, Valid user id.
     */
    int StartAbilityJust(AbilityRequest &abilityRequest, int32_t validUserId);

    /**
     * CallRequestDone, after invoke callRequest, ability will call this interface to return callee.
     *
     * @param token, ability's token.
     * @param callStub, ability's callee.
     */
    void CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub) override;

    /**
     * Release the call between Ability, disconnect session with common ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param element, the element of target service.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ReleaseCall(
        const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element) override;

    std::list<std::shared_ptr<ConnectionRecord>> GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);
    void OnCallConnectDied(std::shared_ptr<CallRecord> callRecord);
    void HandleLoadTimeOut(int64_t abilityRecordId, bool isHalf = false);
    void HandleActiveTimeOut(int64_t abilityRecordId);
    void HandleInactiveTimeOut(int64_t abilityRecordId);
    void HandleForegroundTimeOut(int64_t abilityRecordId, bool isHalf = false);
    void HandleShareDataTimeOut(int64_t uniqueId);
    int32_t GetShareDataPairAndReturnData(std::shared_ptr<AbilityRecord> abilityRecord,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam);

    int32_t StartAbilityByFreeInstall(const Want &want, sptr<IRemoteObject> callerToken, int32_t userId,
        int32_t requestCode);

    int StartAbilityWrap(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int requestCode,
        bool isPendingWantCaller,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isStartAsCaller = false,
        uint32_t specifyTokenId = 0,
        bool isForegroundToRestartApp = false,
        bool isImplicit = false);

    int StartAbilityInner(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int requestCode,
        bool isPendingWantCaller,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isStartAsCaller = false,
        uint32_t specifyTokenId = 0,
        bool isForegroundToRestartApp = false,
        bool isImplicit = false);

    int StartExtensionAbilityInner(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId,
        AppExecFwk::ExtensionAbilityType extensionType,
        bool checkSystemCaller = true,
        bool isImplicit = false,
        bool isDlp = false);

    int RequestModalUIExtensionInner(Want want);

    int PreloadUIExtensionAbilityInner(const Want &want, std::string &bundleName,
        int32_t userId = DEFAULT_INVAL_VALUE);

    int StartAbilityForOptionWrap(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        bool isPendingWantCaller,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        bool isStartAsCaller = false,
        uint32_t callerTokenId = 0,
        bool isImplicit = false,
        bool isCallByShortcut = false);

    int StartAbilityForOptionInner(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        bool isPendingWantCaller,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        bool isStartAsCaller = false,
        uint32_t specifyTokenId = 0,
        bool isImplicit = false,
        bool isCallByShortcut = false);

    int ImplicitStartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE);

    int StartAbilityDetails(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        bool isImplicit = false);

    int ImplicitStartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE);

    int ImplicitStartExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED);

    int StartAbilityAsCallerDetails(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        bool isImplicit = false);

    int ImplicitStartAbilityAsCaller(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE);

    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId = 0);
    void OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want, int32_t requestId = 0);

    void OnStartSpecifiedProcessResponse(const AAFwk::Want &want, const std::string &flag,
        int32_t requestId = 0);
    void OnStartSpecifiedProcessTimeoutResponse(const AAFwk::Want &want, int32_t requestId = 0);

    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override;
    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override;
    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override;
    int GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId);
    void GetAbilityRunningInfo(std::vector<AbilityRunningInfo> &info, std::shared_ptr<AbilityRecord> &abilityRecord);
    void GetExtensionRunningInfo(std::shared_ptr<AbilityRecord> &abilityRecord, const int32_t userId,
        std::vector<ExtensionRunningInfo> &info);

    int GetMissionSaveTime() const;

    /**
     * generate ability request.
     *
     */
    int GenerateAbilityRequest(
        const Want &want,
        int requestCode,
        AbilityRequest &request,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId);

    /**
     * Get mission id by target ability token.
     *
     * @param token target ability token.
     * @return the missionId of target mission.
     */
    int32_t GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * Get ability token by target mission id.
     *
     * @param missionId target missionId.
     * @return the ability token of target mission.
     */
    sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId);

    virtual int StartUser(int userId, sptr<IUserCallback> callback) override;

    virtual int StopUser(int userId, const sptr<IUserCallback> &callback) override;

    virtual int LogoutUser(int32_t userId) override;

    /**
     * Called when client complete dump.
     *
     * @param infos The dump info.
     * @param callerToken The caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken) override;

    virtual int SetMissionContinueState(const sptr<IRemoteObject> &abilityToken,
        const AAFwk::ContinueState &state) override;

#ifdef SUPPORT_SCREEN
    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) override;

    virtual int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;

    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
        bool animationEnabled) override;

    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override;

    virtual void CompleteFirstFrameDrawing(int32_t sessionId) override;

    sptr<IWindowManagerServiceHandler> GetWMSHandler() const;

    virtual int PrepareTerminateAbility(const sptr<IRemoteObject> &token,
        sptr<IPrepareTerminateCallback> &callback) override;

    void HandleFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo);

    void HandleUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo);

    virtual int GetDialogSessionInfo(const std::string &dialogSessionId,
        sptr<DialogSessionInfo> &dialogSessionInfo) override;

    virtual int SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllowed) override;

    int CreateCloneSelectorDialog(AbilityRequest &request, int32_t userId, const std::string &replaceWantString = "");

    void SetTargetCloneIndexInSameBundle(const Want &want, sptr<IRemoteObject> callerToken);

    virtual int RegisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer,
        const std::string &bundleName) override;

    virtual int UnregisterAbilityFirstFrameStateObserver(
        const sptr<IAbilityFirstFrameStateObserver> &observer) override;

    bool GetAnimationFlag();

#endif

    void ClearUserData(int32_t userId);

    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;

    virtual int32_t GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;

    /**
     * Set ability controller.
     *
     * @param abilityController, The ability controller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetAbilityController(const sptr<IAbilityController> &abilityController, bool imAStabilityTest) override;

    /**
     * Is user a stability test.
     *
     * @return Returns true if user is a stability test.
     */
    virtual bool IsRunningInStabilityTest() override;

    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) override;

    virtual int FinishUserTest(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override;

    /**
     * GetTopAbility, get the token of top ability.
     *
     * @param token, the token of top ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetTopAbility(sptr<IRemoteObject> &token) override;

    virtual int CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused) override;

    /**
     * The delegator calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) override;

    /**
     * The delegator calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) override;

    /**
     * Calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) override;

    /**
     * Calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) override;

    bool IsAbilityControllerStart(const Want &want, const std::string &bundleName);

    bool IsAbilityControllerForeground(const std::string &bundleName);

    bool IsAbilityControllerStartById(int32_t missionId);

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability manager service.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAmsService() override;

    /**
     * Block ability.
     *
     * @param abilityRecordId The Ability Record Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAbility(int32_t abilityRecordId) override;

    /**
     * Block app manager service.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService() override;

    /**
     * force timeout ability.
     *
     * @param abilityName.
     * @param state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) override;
    #endif

    bool GetDataAbilityUri(const std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
        const std::string &mainAbility, std::string &uri);

    virtual AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId = true) override;

    virtual AppExecFwk::ElementName GetElementNameByToken(sptr<IRemoteObject> token,
        bool isNeedLocalDeviceId = true) override;

    /**
     * AtomicServiceStatusCallback OnInstallFinished callback.
     *
     * @param resultCode FreeInstall result code.
     * @param want Want has been installed.
     * @param userId User id.
     */
    void OnInstallFinished(int resultCode, const Want &want, int32_t userId);

    /**
     * AtomicServiceStatusCallback OnRemoteInstallFinished callback.
     *
     * @param resultCode FreeInstall result code.
     * @param want Want has been installed.
     * @param userId User id.
     */
    void OnRemoteInstallFinished(int resultCode, const Want &want, int32_t userId);

    /**
     * FreeInstall form remote call.
     *
     * @param want Want need to install.
     * @param callback DMS callback.
     * @param userId User id.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Add FreeInstall Observer
     *
     * @param observer the observer of ability free install start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AddFreeInstallObserver(const sptr<IRemoteObject> &callerToken,
        const sptr<AbilityRuntime::IFreeInstallObserver> &observer) override;

    /**
     * Check the uid is background task uid.
     *
     * @param uid userId.
     * @return Returns whether the uid is background task uid.
     */
    bool IsBackgroundTaskUid(const int uid);

    bool GetLocalDeviceId(std::string& localDeviceId);

    int JudgeAbilityVisibleControl(const AppExecFwk::AbilityInfo &abilityInfo);

    /**
     * Called to update mission snapshot.
     * @param token The target ability.
     * @param pixelMap The snapshot.
     */
#ifdef SUPPORT_SCREEN
    virtual void UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
        const std::shared_ptr<Media::PixelMap> &pixelMap) override;
#endif // SUPPORT_SCREEN
    virtual void EnableRecoverAbility(const sptr<IRemoteObject>& token) override;
    virtual void SubmitSaveRecoveryInfo(const sptr<IRemoteObject>& token) override;
    virtual void ScheduleRecoverAbility(const sptr<IRemoteObject> &token, int32_t reason,
        const Want *want = nullptr) override;

    virtual void ScheduleClearRecoveryPageStack() override;

    /**
     * Called to verify that the MissionId is valid.
     * @param missionIds Query mission list.
     * @param results Output parameters, return results up to 20 query results.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t IsValidMissionIds(
        const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results) override;

    virtual int32_t RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken) override;

    int32_t ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken) override;

    virtual int32_t AcquireShareData(
        const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData) override;
    virtual int32_t ShareDataDone(const sptr<IRemoteObject>& token,
        const int32_t &requestCode, const int32_t &uniqueId, WantParams &wantParam) override;

    bool GetStartUpNewRuleFlag() const;

    std::shared_ptr<AbilityRecord> GetFocusAbility();

    /**
     * Query whether the application of the specified PID and UID has been granted a certain permission
     * @param permission
     * @param pid Process id
     * @param uid
     * @return Returns ERR_OK if the current process has the permission, others on failure.
     */
    virtual int VerifyPermission(const std::string &permission, int pid, int uid) override;

    bool IsDmsAlive() const;

    /**
     * Upgrade app completed event.
     * @param bundleName.
     * @param uid.
     */
    void AppUpgradeCompleted(const std::string &bundleName, int32_t uid);

    /**
     * Record app exit reason.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RecordAppExitReason(const ExitReason &exitReason) override;

    /**
     * Force app exit and record exit reason.
     * @param pid Process id .
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ForceExitApp(const int32_t pid, const ExitReason &exitReason) override;

    /**
     * Record the process exit reason before the process being killed.
     * @param pid The process id.
     * @param exitReason The reason of process exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason) override;

    int32_t GetConfiguration(AppExecFwk::Configuration& config);

    /**
     * Set rootSceneSession by SCB.
     *
     * @param rootSceneSession Indicates root scene session of SCB.
     */
    virtual void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession) override;

    /**
     * Call UIAbility by SCB.
     *
     * @param sessionInfo the session info of the ability to be called.
     * @param isColdStart the session of the ability is or not cold start.
     */
    virtual void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isColdStart) override;

    /**
     * Start specified ability by SCB.
     *
     * @param want Want information.
     */
    virtual void StartSpecifiedAbilityBySCB(const Want &want) override;

    /**
     * Notify sandbox app the result of saving file.
     * @param want Result of saving file, which contains the file's uri if success.
     * @param resultCode Indicates the action's result.
     * @param requestCode Pass the requestCode to match request.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifySaveAsResult(const Want &want, int resultCode, int requestCode) override;

    /**
     * Set sessionManagerService
     * @param sessionManagerService the point of sessionManagerService.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetSessionManagerService(const sptr<IRemoteObject> &sessionManagerService) override;

    /**
     * @brief Register collaborator.
     * @param type collaborator type.
     * @param impl collaborator.
     * @return 0 or else.
    */
    virtual int32_t RegisterIAbilityManagerCollaborator(
        int32_t type, const sptr<IAbilityManagerCollaborator> &impl) override;

    /**
     * @brief Unregister collaborator.
     * @param type collaborator type.
     * @return 0 or else.
    */
    virtual int32_t UnregisterIAbilityManagerCollaborator(int32_t type) override;

    /**
     * @brief Get collaborator.
     * @param type collaborator type.
     * @return nullptr or IAbilityManagerCollaborator stpr.
    */
    sptr<IAbilityManagerCollaborator> GetCollaborator(int32_t type);

    /**
     * get the user id.
     *
     */
    int32_t GetUserId() const;

    virtual int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate) override;

    virtual int32_t KillProcessWithPrepareTerminate(const std::vector<int32_t>& pids) override;

    /**
     * @brief Register auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback) override;

    /**
     * @brief Unregister auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback) override;

    /**
     * @brief Set every application auto start up state.
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetApplicationAutoStartup(const AutoStartupInfo &info) override;

    /**
     * @brief Cancel every application auto start up .
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CancelApplicationAutoStartup(const AutoStartupInfo &info) override;

    /**
     * @brief Query auto startup state all application.
     * @param infoList Output parameters, return auto startup info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList) override;

    /**
     * PrepareTerminateAbilityBySCB, prepare to terminate ability by scb.
     *
     * @param sessionInfo the session info of the ability to start.
     * @param isTerminate the result of ability onPrepareToTermiante.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PrepareTerminateAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isTerminate) override;

    /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) override;

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) override;

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
     * @brief Execute intent.
     * @param key The key of intent executing client.
     * @param callerToken Caller ability token.
     * @param param The Intent execute param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ExecuteIntent(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param) override;

    /**
     * @brief Execute intent.
     * @param abilityRequest The abilityRequest.
     */
    int32_t OnExecuteIntent(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord);

    int32_t StartAbilityWithInsightIntent(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE);

    int32_t StartAbilityByCallWithInsightIntent(const Want &want, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param);

    /**
     * @brief Check if ability controller can start.
     * @param want The want of ability to start.
     * @return Return true to allow ability to start, or false to reject.
     */
    virtual bool IsAbilityControllerStart(const Want &want) override;

    /**
     * @brief Called when insight intent execute finished.
     *
     * @param token ability's token.
     * @param intentId insight intent id.
     * @param result insight intent execute result.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result) override;

    /**
     * @brief Open file by uri.
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @return int The file descriptor.
     */
    virtual int32_t OpenFile(const Uri& uri, uint32_t flag) override;

    /**
     * @brief Set application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override;

    /**
     * @brief Cancel application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override;

    /**
     * @brief Get foreground ui abilities.
     * @param list Foreground ui abilities.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list) override;

    void RemoveLauncherDeathRecipient(int32_t userId);

    /**
     * @brief Update session info.
     * @param sessionInfos The vector of session info.
     */
    virtual int32_t UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, int32_t userId,
        std::vector<int32_t> &sessionIds) override;

    /**
     * @brief Get host info of root caller.
     *
     * @param token The ability token.
     * @param hostInfo The host info of root caller.
     * @param userId The user id.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    int32_t GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token, UIExtensionHostInfo &hostInfo,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * @brief Get ui extension session info
     *
     * @param token The ability token.
     * @param uiExtensionSessionInfo The ui extension session info.
     * @param userId The user id.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Set the enable status for starting and stopping resident processes.
     * The caller application can only set the resident status of the configured process.
     * @param bundleName The bundle name of the resident process.
     * @param enable Set resident process enable status.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetResidentProcessEnabled(const std::string &bundleName, bool enable) override;

    /**
     * @brief Restart app self.
     * @param want The ability type must be UIAbility.
     * @param isAppRecovery True indicates that the app is restarted because of recovery.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RestartApp(const AAFwk::Want &want, bool isAppRecovery = false) override;

    /**
     * @brief Request to display assert fault dialog.
     * @param callback Listen for user operation callbacks.
     * @param wantParams Assert dialog box display information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RequestAssertFaultDialog(
        const sptr<IRemoteObject> &callback, const AAFwk::WantParams &wantParams) override;

    /**
     * @brief Notify the operation status of the user.
     * @param assertFaultSessionId Indicates the request ID of AssertFault.
     * @param userStatus Operation status of the user.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyDebugAssertResult(uint64_t assertFaultSessionId, AAFwk::UserStatus userStatus) override;

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartShortcut(const Want &want, const StartOptions &startOptions) override;

    /**
     * Get ability state by persistent id.
     *
     * @param persistentId, the persistentId of the session.
     * @param state Indicates the ability state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAbilityStateByPersistentId(int32_t persistentId, bool &state) override;

    /**
     * Transfer resultCode & want to ability manager service.
     *
     * @param callerToken Caller ability token.
     * @param requestCode The resultCode of the ability to start.
     * @param want Indicates the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t TransferAbilityResultForExtension(const sptr<IRemoteObject> &callerToken, int32_t resultCode,
        const Want &want) override;

    std::shared_ptr<MissionListManagerInterface> GetMissionListManagerByUserId(int32_t userId);
    std::shared_ptr<MissionListWrap> GetMissionListWrap();

    /**
     * Notify ability manager service frozen process.
     *
     * @param pidList, the pid list of the frozen process.
     * @param uid, the uid of the frozen process.
     */
    virtual void NotifyFrozenProcessByRSS(const std::vector<int32_t> &pidList, int32_t uid) override;

    void HandleRestartResidentProcessDependedOnWeb();

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
        const std::string& abilityName, const std::string& startTime) override;

    int32_t StartUIAbilityByPreInstall(const FreeInstallInfo &taskInfo);

    void NotifySCBToHandleAtomicServiceException(const std::string& sessionId, int errCode,
        const std::string& reason);

    int32_t TerminateMission(int32_t missionId) override;

    // MSG 0 - 20 represents timeout message
    static constexpr uint32_t LOAD_TIMEOUT_MSG = 0;
    static constexpr uint32_t ACTIVE_TIMEOUT_MSG = 1;
    static constexpr uint32_t INACTIVE_TIMEOUT_MSG = 2;
    static constexpr uint32_t TERMINATE_TIMEOUT_MSG = 4;
    static constexpr uint32_t FOREGROUND_TIMEOUT_MSG = 5;
    static constexpr uint32_t BACKGROUND_TIMEOUT_MSG = 6;
    static constexpr uint32_t SHAREDATA_TIMEOUT_MSG = 7;

#ifdef SUPPORT_ASAN
    static constexpr uint32_t LOAD_TIMEOUT = 150000;            // ms
    static constexpr uint32_t ACTIVE_TIMEOUT = 75000;          // ms
    static constexpr uint32_t INACTIVE_TIMEOUT = 7500;         // ms
#else
    static constexpr uint32_t LOAD_TIMEOUT = 10000;            // ms
    static constexpr uint32_t ACTIVE_TIMEOUT = 5000;          // ms
    static constexpr uint32_t INACTIVE_TIMEOUT = 500;         // ms
#endif

    static constexpr uint32_t MIN_DUMP_ARGUMENT_NUM = 2;
    static constexpr uint32_t MAX_WAIT_SYSTEM_UI_NUM = 600;
    static constexpr uint32_t MAX_WAIT_SETTINGS_DATA_NUM = 300;

    enum {
        ABILITY_MOVE_TO_FOREGROUND_CODE = 0,
        ABILITY_MOVE_TO_BACKGROUND_CODE,
        TERMINATE_ABILITY_CODE
    };

    friend class UserController;

protected:
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) override;
    int GetUidByBundleName(std::string bundleName);

    void OnAppStateChanged(const AppInfo &info) override;

    void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) override;

    void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) override;

private:
    int TerminateAbilityWithFlag(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr, bool flag = true);
    /**
     * initialization of ability manager service.
     *
     */
    bool Init();
    /**
     * initialization of u0 user.
     *
     */
    void InitU0User();
    void InitStartAbilityChain();
    /**
     * start highest priority ability.
     *
     */
    void StartHighestPriorityAbility(int32_t userId, bool isBoot);
    /**
     * connet bms.
     *
     */
    void ConnectBmsService();

    /**
     * Determine whether it is a system APP
     *
     */
    bool IsSystemUiApp(const AppExecFwk::AbilityInfo &info) const;
    /**
     * Init parameters from the global
     *
     */
    void InitGlobalConfiguration();

    std::shared_ptr<AppExecFwk::BundleMgrHelper> GetBundleManager();

    sptr<OHOS::AppExecFwk::IAppMgr> GetAppMgr();

    int StartRemoteAbility(const Want &want, int requestCode, int32_t validUserId,
        const sptr<IRemoteObject> &callerToken);
    int StartUIAbilityBySCBDefault(sptr<SessionInfo> sessionInfo, uint32_t sceneFlag, bool &isColdStart);
    int StartUIAbilityByPreInstallInner(sptr<SessionInfo> sessionInfo, uint32_t specifyTokenId,
        uint32_t sceneFlag, bool &isColdStart);
    int32_t PreStartInner(const FreeInstallInfo& taskInfo);
    void RemovePreStartSession(const std::string& sessionId);

    int ConnectLocalAbility(
        const Want &want,
        const int32_t userId,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ExtensionAbilityType extensionType,
        const sptr<SessionInfo> &sessionInfo = nullptr,
        bool isQueryExtensionOnly = false,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr);
    int DisconnectLocalAbility(const sptr<IAbilityConnection> &connect);
    int ConnectRemoteAbility(Want &want, const sptr<IRemoteObject> &callerToken, const sptr<IRemoteObject> &connect);
    int DisconnectRemoteAbility(const sptr<IRemoteObject> &connect);
    int PreLoadAppDataAbilities(const std::string &bundleName, const int32_t userId);
    void PreLoadAppDataAbilitiesTask(const std::string &bundleName, const int32_t userId);
    void UpdateAsCallerSourceInfo(Want& want, sptr<IRemoteObject> asCallerSourceToken, sptr<IRemoteObject> callerToken);
    void UpdateAsCallerInfoFromToken(Want& want, sptr<IRemoteObject> asCallerSourceToken);
    void UpdateAsCallerInfoFromCallerRecord(Want& want, sptr<IRemoteObject> callerToken);
    void UpdateAsCallerInfoFromDialog(Want& want);
    void UpdateCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken);
    void UpdateCallerInfoFromToken(Want& want, const sptr<IRemoteObject> &token);
    int StartAbilityPublicPrechainCheck(StartAbilityParams &params);
    int StartAbilityPrechainInterceptor(StartAbilityParams &params);
    bool StartAbilityInChain(StartAbilityParams &params, int &result);

    bool CheckIfOperateRemote(const Want &want);
    std::string AnonymizeDeviceId(const std::string& deviceId);
    bool VerificationToken(const sptr<IRemoteObject> &token);
    void RequestPermission(const Want *resultWant);

    bool CheckIsRemote(const std::string& deviceId);
    int GetRemoteMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos);
    int GetRemoteMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo);
    int32_t GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& missionSnapshot);
    int StartRemoteAbilityByCall(const Want &want, const sptr<IRemoteObject> &callerToken,
        const sptr<IRemoteObject> &connect);
    int ReleaseRemoteAbility(const sptr<IRemoteObject> &connect, const AppExecFwk::ElementName &element);

    void DumpInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionInner(const std::string &args, std::vector<std::string> &info);
    void DumpStateInner(const std::string &args, std::vector<std::string> &info);
    void DataDumpStateInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionListInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionInfosInner(const std::string &args, std::vector<std::string> &info);

    bool JudgeMultiUserConcurrency(const int32_t userId);
    /**
     * dumpsys info
     *
     */
    void DumpSysInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysMissionListInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysMissionListInnerBySCB(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysAbilityInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysAbilityInnerBySCB(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysStateInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysPendingInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysProcess(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpUIExtensionRootHostRunningInfos(pid_t pid, std::vector<std::string> &info);
    void DumpUIExtensionProviderRunningInfos(pid_t pid, std::vector<std::string> &info);
    void DataDumpSysStateInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    ErrCode ProcessMultiParam(std::vector<std::string>& argsStr, std::string& result);
    void ShowHelp(std::string& result);
    void ShowIllegalInfomation(std::string& result);
    int Dump(const std::vector<std::u16string>& args, std::string& result);

    // multi user
    void StartFreezingScreen();
    void StopFreezingScreen();
    void UserStarted(int32_t userId);
    void SwitchToUser(int32_t userId);
    void SwitchToUser(int32_t oldUserId, int32_t userId, sptr<IUserCallback> callback);
    void SwitchManagers(int32_t userId, bool switchUser = true);
    void StartUserApps();
    void PauseOldUser(int32_t userId);
    void PauseOldMissionListManager(int32_t userId);
    void PauseOldConnectManager(int32_t userId);
    bool IsSystemUI(const std::string &bundleName) const;

    bool VerificationAllToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<DataAbilityManager> GetCurrentDataAbilityManager();
    std::shared_ptr<DataAbilityManager> GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token);
    std::unordered_map<int, std::shared_ptr<AbilityConnectManager>> GetConnectManagers();
    std::shared_ptr<AbilityConnectManager> GetCurrentConnectManager();
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByUserId(int32_t userId);
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<PendingWantManager> GetCurrentPendingWantManager();
    std::shared_ptr<PendingWantManager> GetPendingWantManagerByUserId(int32_t userId);
    std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> GetMissionListManagers();
    std::shared_ptr<MissionListManagerInterface> GetCurrentMissionListManager();
    std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> GetUIAbilityManagers();
    std::shared_ptr<UIAbilityLifecycleManager> GetCurrentUIAbilityManager();
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUid(int32_t uid);
    bool JudgeSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord);
    bool IsAppSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord);

    int32_t GetValidUserId(const int32_t userId);

    int DelegatorMoveMissionToFront(int32_t missionId);

    bool IsNeedTimeoutForTest(const std::string &abilityName, const std::string &state) const;

    void StartResidentApps();

    void StartAutoStartupApps();
    void RetryStartAutoStartupApps(const std::vector<AutoStartupInfo> &infoList, int32_t retryCount);
    void SubscribeScreenUnlockedEvent();
    void UnSubscribeScreenUnlockedEvent();
    void RetrySubscribeScreenUnlockedEvent(int32_t retryCount);
    void RemoveScreenUnlockInterceptor();

    int VerifyAccountPermission(int32_t userId);

    int CheckStaticCfgPermissionForAbility(const AppExecFwk::AbilityInfo &abilityInfo, uint32_t tokenId);

    int CheckStaticCfgPermissionForSkill(const AppExecFwk::AbilityRequest &abilityRequest, uint32_t tokenId);

    bool CheckOneSkillPermission(const AppExecFwk::Skill &skill, uint32_t tokenId);

    int CheckStaticCfgPermission(const AppExecFwk::AbilityRequest &abilityRequest, bool isStartAsCaller,
        uint32_t callerTokenId, bool isData = false, bool isSaCall = false, bool isImplicit = false);

    int CheckPermissionForUIService(const Want &want, const AbilityRequest &abilityRequest);

    bool GetValidDataAbilityUri(const std::string &abilityInfoUri, std::string &adjustUri);

    int GenerateExtensionAbilityRequest(const Want &want, AbilityRequest &request,
        const sptr<IRemoteObject> &callerToken, int32_t userId);
    int32_t InitialAbilityRequest(AbilityRequest &request, const StartAbilityInfo &abilityInfo) const;
    int CheckOptExtensionAbility(const Want &want, AbilityRequest &abilityRequest,
        int32_t validUserId, AppExecFwk::ExtensionAbilityType extensionType, bool isImplicit = false);

    void SubscribeBackgroundTask();

    void UnSubscribeBackgroundTask();

    void SubscribeBundleEventCallback();

    void UnsubscribeBundleEventCallback();

    void ReportAbilitStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);

    void ReportAbilitAssociatedStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo, int64_t type,
        const sptr<IRemoteObject> &callerToken);

    void ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo, sptr<IRemoteObject> callerToken);

    void ReportAppRecoverResult(const int32_t appId, const AppExecFwk::ApplicationInfo &appInfo,
        const std::string& abilityName, const std::string& result);

    void AppRecoverKill(pid_t pid, int32_t reason);

    int32_t GenerateEmbeddableUIAbilityRequest(const Want &want, AbilityRequest &request,
        const sptr<IRemoteObject> &callerToken, int32_t userId);

    bool GenerateDataAbilityRequestByUri(const std::string& dataAbilityUri,
        AbilityRequest &abilityRequest, sptr<IRemoteObject> callerToken, int32_t userId);

    /**
     * Check if Caller is allowed to start ServiceAbility(FA) or ServiceExtension(Stage) or DataShareExtension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start Service.
     */
    int CheckCallServicePermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start DataAbility(FA)
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start DataAbility.
     */
    int CheckCallDataAbilityPermission(AbilityRequest &abilityRequest, bool isShell, bool IsSACall = false);

    /**
     * Check if Caller is allowed to start ServiceExtension(Stage) or DataShareExtension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start ServiceExtension.
     */
    int CheckCallServiceExtensionPermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start other Extension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start OtherExtension.
     */
    int CheckCallOtherExtensionPermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start ServiceAbility(FA).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start ServiceAbility.
     */
    int CheckCallServiceAbilityPermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start PageAbility(FA) or Ability(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start Ability.
     */
    int CheckCallAbilityPermission(const AbilityRequest &abilityRequest, uint32_t specifyTokenId = 0,
        bool isCallByShortcut = false);

    /**
     * Check if Caller is allowed to start Ability(Stage) by call.
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start Ability by call.
     */
    int CheckStartByCallPermission(const AbilityRequest &abilityRequest);

    /**
     * @brief Check some specified uiextension type should be a system app.
     * Consider expanding it to table-driven in the future.
     *
     * @param abilityRequest The ability request.
     * @return Returns ERR_OK when allowed, others when check failed.
     */
    int CheckUIExtensionPermission(const AbilityRequest &abilityRequest);

    /**
     * Judge if Caller-Application is in background state.
     *
     * @param abilityRequest, abilityRequest.
     * @param isBackgroundCall, Indicates the Caller-Application state.
     *                          TRUE: The Caller-Application is not in focus and not in foreground state.
     *                          FALSE: The Caller-Application is in focus or in foreground state.
     * @return Returns ERR_OK on check success, others on check failure.
     */
    int IsCallFromBackground(const AbilityRequest &abilityRequest, bool &isBackgroundCall, bool isData = false);

    bool IsTargetPermission(const Want &want) const;

    bool IsDelegatorCall(const AppExecFwk::RunningProcessInfo &processInfo, const AbilityRequest &abilityRequest) const;

    bool CheckNewRuleSwitchState(const std::string &param);

    void UpdateFocusState(std::vector<AbilityRunningInfo> &info);

    AAFwk::PermissionVerification::VerificationInfo CreateVerificationInfo(
        const AbilityRequest &abilityRequest, bool isData = false, bool isShell = false, bool isSA = false);

    int AddStartControlParam(Want &want, const sptr<IRemoteObject> &callerToken);

    AAFwk::EventInfo BuildEventInfo(const Want &want, int32_t userId);

#ifdef WITH_DLP
    int CheckDlpForExtension(
        const Want &want, const sptr<IRemoteObject> &callerToken,
        int32_t userId, AAFwk::EventInfo &eventInfo, const EventName &eventName);
#endif // WITH_DLP

    void InitStartupFlag();

    inline bool IsCrossUserCall(int32_t userId)
    {
        return (userId != INVALID_USER_ID && userId != U0_USER_ID && userId != GetUserId());
    }

    int32_t RequestDialogServiceInner(const Want &want, const sptr<IRemoteObject> &callerToken,
        int requestCode, int32_t userId);

    bool CheckCallingTokenId(const std::string &bundleName, int32_t userId = INVALID_USER_ID, int32_t appIndex = 0);
    bool IsCallerSceneBoard();

    void ReleaseAbilityTokenMap(const sptr<IRemoteObject> &token);

    bool CheckPrepareTerminateEnable();

    bool CheckCollaboratorType(int32_t type);

    bool CheckUserIdActive(int32_t userId);

    int32_t CheckProcessOptions(const Want &want, const StartOptions &startOptions, int32_t userId);

    void GetConnectManagerAndUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo,
        std::shared_ptr<AbilityConnectManager> &connectManager, std::shared_ptr<AbilityRecord> &targetAbility);

    virtual int RegisterSessionHandler(const sptr<IRemoteObject> &object) override;

    /**
     * Start switch user dialog Extension ability.
     */
    void StartSwitchUserDialog();

    /**
     * Start switch user dialog inner.
     * @param want, The want of the dialog box to start.
     * @param startUserId, The userId who wants to start the dialog box.
     */
    void StartSwitchUserDialogInner(const Want &want, int32_t startUserId);

    /**
     * Stop switch user dialog Extension ability.
     */
    void StopSwitchUserDialog();

    /**
     * Stop switch user dialog inner.
     * @param want, The want of the dialog box to stop.
     * @param stopUserId, The userId who wants to stop the dialog box.
     */
    void StopSwitchUserDialogInner(const Want &want, const int32_t stopUserId);

    void SetPickerElementName(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId);

    void SetAutoFillElementName(const sptr<SessionInfo> &extensionSessionInfo);

    /**
     * @brief Start extension ability with insight intent
     * @param want, the want of the ability to start.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartExtensionAbilityWithInsightIntent(const Want &want,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED);

    bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord,
        const int32_t oriValidUserId);

    void InitInterceptor();
    void InitPushTask();
    void InitDeepLinkReserve();
    void InitDefaultRecoveryList();

    bool CheckSenderWantInfo(int32_t callerUid, const WantSenderInfo &wantSenderInfo);

    int32_t UninstallAppInner(const std::string &bundleName, const int32_t uid, int32_t appIndex, const bool isUpgrade,
        const std::string &exitMsg);

    int32_t GetMissionIdByAbilityTokenInner(const sptr<IRemoteObject> &token);
    bool CheckCallerIsDmsProcess();

    void WaitBootAnimationStart();

    int32_t SignRestartAppFlag(int32_t userId, const std::string &bundleName, bool isAppRecovery = false);
    int32_t CheckRestartAppWant(const AAFwk::Want &want);

    int32_t CheckDebugAssertPermission();
    std::shared_ptr<AbilityDebugDeal> ConnectInitAbilityDebugDeal();

    int StartUIAbilityForOptionWrap(const Want &want, const StartOptions &options, sptr<IRemoteObject> callerToken,
        bool isPendingWantCaller, int32_t userId, int requestCode, uint32_t callerTokenId = 0, bool isImplicit = false,
        bool isCallByShortcut = false);

    int32_t SetBackgroundCall(const AppExecFwk::RunningProcessInfo &processInfo,
        const AbilityRequest &abilityRequest, bool &isBackgroundCall) const;

    void GetRunningMultiAppIndex(const std::string &bundleName, int32_t uid, int32_t &appIndex);
    ErrCode ConvertToExplicitWant(Want& want);

    int CheckUIExtensionUsage(AppExecFwk::UIExtensionUsage uiExtensionUsage,
        AppExecFwk::ExtensionAbilityType extensionType);

    bool CheckUIExtensionCallerIsForeground(const AbilityRequest &abilityRequest);

    int CheckExtensionCallPermission(const Want& want, const AbilityRequest& abilityRequest);

    int CheckServiceCallPermission(const AbilityRequest& abilityRequest,
        const AppExecFwk::AbilityInfo& abilityInfo);

    int CheckBrokerCallPermission(const AbilityRequest& abilityRequest,
        const AppExecFwk::AbilityInfo& abilityInfo);

    int CheckAbilityCallPermission(const AbilityRequest& abilityRequest,
        const AppExecFwk::AbilityInfo& abilityInfo, uint32_t specifyTokenId);

    int CheckCallPermission(const Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
        const AbilityRequest& abilityRequest, bool isForegroundToRestartApp,
        bool isSendDialogResult, uint32_t specifyTokenId,
        const std::string& callerBundleName);

    int StartAbilityByConnectManager(const Want& want, const AbilityRequest& abilityRequest,
        const AppExecFwk::AbilityInfo& abilityInfo, int validUserId, sptr<IRemoteObject> callerToken);

    int PreStartFreeInstall(const Want &want, sptr<IRemoteObject> callerToken,
        uint32_t specifyTokenId, bool isStartAsCaller, Want &localWant);

    void ReportCleanSession(const sptr<SessionInfo> &sessionInfo,
        const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t errCode);

    constexpr static int REPOLL_TIME_MICRO_SECONDS = 1000000;
    constexpr static int WAITING_BOOT_ANIMATION_TIMER = 5;

    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AbilityEventHandler> eventHandler_;
    ServiceRunningState state_;
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgrHelper_;
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr_ { nullptr };

    std::shared_ptr<FreeInstallManager> freeInstallManager_;

    std::shared_ptr<SubManagersHelper> subManagersHelper_;

    std::shared_ptr<UserController> userController_;
    sptr<AppExecFwk::IAbilityController> abilityController_ = nullptr;
    bool controllerIsAStabilityTest_ = false;
    ffrt::mutex globalLock_;
    ffrt::mutex bgtaskObserverMutex_;
    ffrt::mutex abilityTokenLock_;
    ffrt::mutex preStartSessionMapLock_;

    std::multimap<std::string, std::string> timeoutMap_;
    std::map<std::string, sptr<SessionInfo>> preStartSessionMap_;

    static sptr<AbilityManagerService> instance_;
    int32_t uniqueId_ = 0;
    std::map<int32_t, std::pair<int64_t, const sptr<IAcquireShareDataCallback>>> iAcquireShareDataMap_;
    // first is callstub, second is ability token
    std::map<sptr<IRemoteObject>, sptr<IRemoteObject>> callStubTokenMap_;
#ifdef SUPPORT_GRAPHICS
    sptr<WindowFocusChangedListener> focusListener_;
#endif // SUPPORT_GRAPHICS
    // Component StartUp rule switch
    bool startUpNewRule_ = true;
    /** It only takes effect when startUpNewRule_ is TRUE
     *  TRUE: When Caller-Application is Launcher or SystemUI, use old rule.
     *  FALSE: Apply new rule to all application
     */
    bool newRuleExceptLauncherSystemUI_ = true;
    /** Indicates the criteria for judging whether the Caller-Application is in the background
     *  TRUE: Determine the state by AAFwk::AppState::FOREGROUND.
     *  FALSE: Determine the state by AppExecFwk::AppProcessState::APP_STATE_FOCUS.
     */
    bool backgroundJudgeFlag_ = true;
    /** The applications in white list can associatedWakeUp
     *  TRUE: white list enable.
     *  FALSE: white list unable.
     */
    bool whiteListassociatedWakeUpFlag_ = true;

    std::shared_ptr<AbilityRuntime::AbilityManagerEventSubscriber> screenSubscriber_;

    std::shared_ptr<AbilityAutoStartupService> abilityAutoStartupService_;

    std::map<std::string, std::list<std::string>> whiteListMap_;

    std::list<std::string> exportWhiteList_;

    bool ShouldPreventStartAbility(const AbilityRequest &abilityRequest);

    void PrintStartAbilityInfo(AppExecFwk::AbilityInfo callerInfo, AppExecFwk::AbilityInfo calledInfo);

    bool IsInWhiteList(const std::string &callerBundleName, const std::string &calleeBundleName,
        const std::string &calleeAbilityName);

    bool isParamStartAbilityEnable_ = false;

    std::string GetConfigFileAbsolutePath(const std::string &relativePath);

    int32_t ParseJsonValueFromFile(nlohmann::json &value, const std::string& fullPath);

    bool ConvertFullPath(const std::string& partialPath, std::string& fullPath);

    bool GetJsonFromFile(const char *filePath, Json::Value &root);

    bool ParseJsonFromBoot(nlohmann::json jsonObj, const std::string &relativePath,
        const std::string &WHITE_LIST);

    void CloseAssertDialog(const std::string &assertSessionId);

    void SetReserveInfo(const std::string &linkString, AbilityRequest& abilityRequest);

    void ReportPreventStartAbilityResult(const AppExecFwk::AbilityInfo &callerAbilityInfo,
        const AppExecFwk::AbilityInfo &abilityInfo);

    void SetAbilityRequestSessionInfo(AbilityRequest &abilityRequest, AppExecFwk::ExtensionAbilityType extensionType);
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::shared_ptr<BackgroundTaskObserver> bgtaskObserver_;
#endif

    sptr<AbilityBundleEventCallback> abilityBundleEventCallback_;

#ifdef SUPPORT_SCREEN
    int32_t ShowPickerDialog(const Want& want, int32_t userId, const sptr<IRemoteObject> &token);
    bool CheckWindowMode(int32_t windowMode, const std::vector<AppExecFwk::SupportWindowMode>& windowModes) const;
    void InitFocusListener();
    void RegisterFocusListener();
    void InitPrepareTerminateConfig();
    std::shared_ptr<ImplicitStartProcessor> implicitStartProcessor_;
    sptr<IWindowManagerServiceHandler> wmsHandler_;
    bool isAnimationEnabled_ = true; //only use on mission list
#endif
    std::shared_ptr<AbilityInterceptorExecuter> interceptorExecuter_;
    std::shared_ptr<AbilityInterceptorExecuter> afterCheckExecuter_;

    std::unordered_map<int32_t, int64_t> appRecoveryHistory_; // uid:time
    bool isPrepareTerminateEnable_ = false;
    std::multimap<int, std::shared_ptr<StartAbilityHandler>, std::greater<int>> startAbilityChain_;

    ffrt::mutex collaboratorMapLock_;
    std::unordered_map<int32_t, sptr<IAbilityManagerCollaborator>> collaboratorMap_;

    ffrt::mutex abilityDebugDealLock_;
    std::shared_ptr<AbilityDebugDeal> abilityDebugDeal_;
    std::shared_ptr<AppExitReasonHelper> appExitReasonHelper_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
