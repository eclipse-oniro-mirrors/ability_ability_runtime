/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "ui_extension_ability_manager.h"
#include "common_extension_manager.h"
#include "ability_debug_deal.h"
#include "ability_event_handler.h"
#include "ability_info.h"
#include "ability_manager_event_subscriber.h"
#include "ability_manager_stub.h"
#include "ability_keep_alive_service.h"
#include "ability_start_with_wait_observer_utils.h"
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
#include "insight_intent_event_mgr.h"
#include "intent_exemption_info.h"
#include "iremote_object.h"
#include "kiosk_manager.h"
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
#ifdef SUPPORT_GRAPHICS
#include "implicit_start_processor.h"
#include "system_dialog_scheduler.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
class IStatusBarDelegate;
struct ExtractInsightIntentGenericInfo;
struct LinkIntentParamMapping;
}
namespace Rosen {
class FocusChangeInfo;
class WindowVisibilityInfo;
}

namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr int32_t INVALID_USER_ID = -1;
constexpr const char* KEY_SESSION_ID = "com.ohos.param.sessionId";
constexpr const char* KEY_REQUEST_ID = "com.ohos.param.requestId";
using OHOS::AppExecFwk::IAbilityController;
struct StartAbilityInfo;
struct StartAbilityWrapParam;
class WindowFocusChangedListener;
class WindowVisibilityChangedListener;
class PreloadManagerService;
class StartOptionsUtils;

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
    friend class PreloadManagerService;
    friend class StartOptionsUtils;
    friend class UIAbilityLifecycleManager;
    static std::shared_ptr<AbilityManagerService> GetPubInstance();

    void OnStart() override;
    void OnStop() override;

    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    ServiceRunningState QueryServiceState() const;

    /**
     * StartAbilityWithServiceMatch with param, send param to ability manager service.
     *
     * @param param, the param of the ability to start.
     * @param requestCode, Ability request code.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartAbilityWithServiceMatch(const InsightIntentExecuteParam &param,
        int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * StartSelfUIAbility with want, start self uiability only on 2-in-1 devices.
     *
     * @param want, the want of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartSelfUIAbility(const Want &want) override;

    /**
     * StartSelfUIAbility with want and startOptions, start self uiability only on 2-in-1 devices.
     *
     * @param want, the want of the ability to start.
     * @param options, the startOptions of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartSelfUIAbilityWithStartOptions(const Want &want,
        const StartOptions &options) override;

    /**
     * Starts self UIAbility with start options and receives the process ID. Supported only on 2-in-1 devices.
     *
     * @param want, the want of the ability to start.
     * @param options, the startOptions of the ability to start.
     * @param callbackId, the id of the callback to get target process id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartSelfUIAbilityWithPidResult(const Want &want, StartOptions &options, uint64_t callbackId) override;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param requestCode, Ability request code.
     * @param userId, Designation User ID.
     * @param specifiedFullTokenId, The specified full token ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want, int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = DEFAULT_INVAL_VALUE,
        uint64_t specifiedFullTokenId = 0) override;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @param specifiedFullTokenId, The specified full token ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        uint64_t specifiedFullTokenId = 0) override;

    /**
     * Start Self UIAbility In Current Process.
     * @param want Ability want.
     * @param specifiedFlag specified flag.
     * @param startOptions Indicates the options used to start.
     * @param hasOptions Is have start options.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode StartSelfUIAbilityInCurrentProcess(const Want &want, const std::string &specifiedFlag,
        const AAFwk::StartOptions &startOptions, bool hasOptions, sptr<IRemoteObject> callerToken) override;

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
     * Start ui session ability with windowId and want.
     *
     * @param primaryWindowId the id of window.
     * @param secondaryWant the want of the ability to start.
     * @param callerToken current caller ability token.
     * @return Returns ERR_OK if success.
     */
    int32_t StartUIAbilitiesInSplitWindowMode(int32_t primaryWindowId, const AAFwk::Want &secondaryWant,
        sptr<IRemoteObject> callerToken) override;
    /**
     * Start UI abilities simultaneously.
     *
     * @param wantList a list of want to start UI abilities.
     * @param requestKey, The unique key of this StartUIAbilities request.
     * @param callerToken current caller ability token.
     * @return Returns ERR_OK if success.
     */
    ErrCode StartUIAbilities(const std::vector<AAFwk::Want> &wantList,
        const std::string &requestKey, sptr<IRemoteObject> callerToken) override;

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
     * Start ui ability
     *
     * @param want the want of the ability to start.
     * @param callerToken caller ability token.
     * @param specifyTokenId The Caller ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityOnlyUIAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint32_t specifyTokenId) override;

    /**
     * Open link of ability and atomic service.
     *
     * @param want Ability want.
     * @param callerToken Caller ability token.
     * @param userId User ID.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t OpenLink(const Want &want, sptr<IRemoteObject> callerToken, int32_t userId = DEFAULT_INVAL_VALUE,
        int32_t requestCode = DEFAULT_INVAL_VALUE, bool hideFailureTipDialog = false) override;

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
    virtual int32_t StartExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED) override;

    /**
     * Request modal UIExtension with want, send want to ability manager service.
     *
     * @param want, the want contains ability info about caller and called.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RequestModalUIExtension(const Want &want) override;

    int PreloadUIExtensionAbility(const Want &want, std::string &hostBundleName,
        int32_t userId = DEFAULT_INVAL_VALUE, int32_t hostPid = DEFAULT_INVAL_VALUE,
        int32_t requestCode = DEFAULT_INVAL_VALUE) override;

    int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord, pid_t &hostPid);

    /**
     * Change the visibility state of an UIAbility.
     *
     * @param token The destination UIAbility.
     * @param isShow The wanted state, show or hide.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow) override;

    /**
     * Change the visibility state of an UIAbility by SCB.
     *
     * @param sessionInfo The destination UIAbility.
     * @param isShow The wanted state, show or hide.
     * @return Returns ERR_OK on success, others on failure.
     */
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
     * @param params start parameters.
     * @param isColdStart the session info of the ability is or not cold start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo, AbilityRuntime::StartParamsBySCB &params,
        bool &isColdStart) override;

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
     * TerminateUIServiceExtensionAbility, terminate the UIServiceExtensionAbility.
     *
     * @param token, the token of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t TerminateUIServiceExtensionAbility(const sptr<IRemoteObject> &token) override;

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
     * CloseUIExtensionAbilityBySCB, terminate the specified ui extension ability by SCB.
     *
     * @param token the ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CloseUIExtensionAbilityBySCB(const sptr<IRemoteObject> token) override;

    /**
     *  CloseUIAbilityBySCB, close the special ability by scb.
     *
     * @param sessionInfo the session info of the ability to terminate.
     * @param sceneFlag the reason info of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool isUserRequestedExit,
        uint32_t sceneFlag = 0) override;

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
    virtual int32_t ConnectAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    virtual int32_t ConnectAbilityCommon(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isQueryExtensionOnly = false,
        uint64_t specifiedFullTokenId = 0,
        int32_t loadTimeout = 0) override;

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
     * @param sessionId, represent a continuation.
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

    virtual int32_t SuspendExtensionAbility(sptr<IAbilityConnection> connect) override;

    virtual int32_t ResumeExtensionAbility(sptr<IAbilityConnection> connect) override;

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
     * @param sceneFlag the reason info of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CleanUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool isUserRequestedExit,
        uint32_t sceneFlag = 0) override;

    std::shared_ptr<TaskHandlerWrap> GetTaskHandler() const
    {
        return taskHandler_;
    }

    int32_t GetRfd() const
    {
        return rFd_;
    }
    int32_t GetWfd() const
    {
        return wFd_;
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
    virtual int KillProcess(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0,
        const std::string& reason = "Abilityms::KillProcess") override;

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
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken, int32_t uid = -1) override;

    /**
     * @brief Send want sender.
     * @param target The target want sender to be triggered.
     * @param senderInfo The sender information containing result data and callbacks.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendWantSender(sptr<IWantSender> target, SenderInfo &senderInfo) override;

    /**
     * @brief Send local pending want without IPC.
     * @param senderInfo The sender information containing result data and callbacks.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendLocalWantSender(const SenderInfo &senderInfo) override;

    /**
     * @brief Cancel a pending want sender.
     * @param sender The want sender to be cancelled.
     */
    virtual void CancelWantSender(const sptr<IWantSender> &sender) override;

    /**
     * @brief Get the UID of a pending want sender.
     * @param target The target want sender.
     * @return Returns the UID on success, -1 on failure.
     */
    virtual int GetPendingWantUid(const sptr<IWantSender> &target) override;

    /**
     * @brief Get the user ID of a pending want sender.
     * @param target The target want sender.
     * @return Returns the user ID on success, -1 on failure.
     */
    virtual int GetPendingWantUserId(const sptr<IWantSender> &target) override;

    /**
     * @brief Get the bundle name of a pending want sender.
     * @param target The target want sender.
     * @return Returns the bundle name.
     */
    virtual std::string GetPendingWantBundleName(const sptr<IWantSender> &target) override;

    /**
     * @brief Get the request code of a pending want sender.
     * @param target The target want sender.
     * @return Returns the request code on success, -1 on failure.
     */
    virtual int GetPendingWantCode(const sptr<IWantSender> &target) override;

    /**
     * @brief Get the type of a pending want sender.
     * @param target The target want sender.
     * @return Returns the type on success, -1 on failure.
     */
    virtual int GetPendingWantType(const sptr<IWantSender> &target) override;

    /**
     * @brief Register a cancel listener for pending want sender.
     * @param sender The want sender to listen on.
     * @param receiver The callback receiver for cancel event.
     */
    virtual void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override;

    /**
     * @brief Unregister a cancel listener from pending want sender.
     * @param sender The want sender.
     * @param receiver The callback receiver to be unregistered.
     */
    virtual void UnregisterCancelListener(
        const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override;

    /**
     * @brief Get the pending want from a want sender.
     * @param target The target want sender.
     * @param want The output pending want.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) override;

    /**
     * @brief Get the want sender information.
     * @param target The target want sender.
     * @param info The output want sender information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) override;

    /**
     * @brief Register an observer for connection state changes.
     * @param observer The connection observer to be registered.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer) override;

    /**
     * @brief Unregister a connection observer.
     * @param observer The connection observer to be unregistered.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer) override;

#ifdef WITH_DLP
    /**
     * @brief Get DLP (Data Loss Prevention) connection information.
     * @param infos The output DLP connection information list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos) override;
#endif // WITH_DLP

    /**
     * @brief Get ability connection data.
     * @param connectionData The output connection data list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData) override;

    /**
     * @brief Cancel want sender by specified flags.
     * @param sender The want sender to be cancelled.
     * @param flags The flags to control cancellation behavior.
     */
    virtual void CancelWantSenderByFlags(const sptr<IWantSender> &sender, uint32_t flags) override;

    /**
     * @brief Lock a mission for cleanup operation.
     * @param missionId The ID of the mission to be locked.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int LockMissionForCleanup(int32_t missionId) override;

    /**
     * @brief Unlock a mission after cleanup.
     * @param missionId The ID of the mission to be unlocked.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnlockMissionForCleanup(int32_t missionId) override;

    /**
     * @brief Set the locked state of a session.
     * @param sessionId The ID of the session.
     * @param lockedState The locked state to be set.
     */
    virtual void SetLockedState(int32_t sessionId, bool lockedState) override;

    /**
     * @brief Register a mission listener.
     * @param listener The mission listener to be registered.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    /**
     * @brief Unregister a mission listener.
     * @param listener The mission listener to be unregistered.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    /**
     * @brief Get all mission information within specified device.
     * @param deviceId The target device ID.
     * @param numMax The maximum number of missions to retrieve.
     * @param missionInfos The output mission information list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override;

    /**
     * @brief Get specific mission information.
     * @param deviceId The target device ID.
     * @param missionId The ID of the mission.
     * @param missionInfo The output mission information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo) override;

    /**
     * @brief Clean a specific mission.
     * @param missionId The ID of the mission to be cleaned.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CleanMission(int32_t missionId) override;

    /**
     * @brief Clean all missions.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CleanAllMissions() override;

    /**
     * @brief Move a specific mission to front.
     * @param missionId The ID of the mission.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToFront(int32_t missionId) override;

    /**
     * @brief Move a specific mission to front with start options.
     * @param missionId The ID of the mission.
     * @param startOptions The start options for the mission.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) override;

    /**
     * @brief Move multiple missions to foreground.
     * @param missionIds The list of mission IDs to be moved.
     * @param topMissionId The ID of the top mission after moving.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override;

    /**
     * @brief Move multiple missions to background.
     * @param missionIds The list of mission IDs to be moved.
     * @param result The output list of moved mission IDs.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
        std::vector<int32_t>& result) override;

    /**
     * @brief Get mission ID by ability token.
     * @param token The ability token.
     * @return Returns the mission ID on success, -1 on failure.
     */
    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) override;

    /**
     * @brief Get ability token by call stub object.
     * @param callStub The call stub object.
     * @param token The output ability token.
     */
    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) override;

    /**
     * @brief Start synchronizing remote missions with specified device.
     * @param devId The target device ID.
     * @param fixConflict Whether to fix conflict when syncing.
     * @param tag The synchronization tag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;

    /**
     * @brief Stop synchronizing remote missions with specified device.
     * @param devId The target device ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopSyncRemoteMissions(const std::string& devId) override;

    /**
     * @brief Get application memory size.
     * @return Returns the memory size in bytes, or 0 on failure.
     */
    virtual int GetAppMemorySize() override;

    /**
     * @brief Check if the device is RAM constrained.
     * @return Returns true if RAM constrained, false otherwise.
     */
    virtual bool IsRamConstrainedDevice() override;

    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken Indicates the caller's identity
     * @param accountId Indicates the account to start.
     * @param isSilent, whether show window when start fail by interceptorExecuter.
     * @param promotePriority, whether to promote priority for sa.
     * @param isVisible, whether show window when start fail by afterCheckExecuter.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId = DEFAULT_INVAL_VALUE,
        bool isSilent = false, bool promotePriority = false, bool isVisible = false) override;

    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken Indicates the caller's identity
     * @param accountId Indicates the account to start.
     * @param errMsg Out parameter, indicates the failed reason.
     * @param isSilent, whether show window when start fail by interceptorExecuter.
     * @param promotePriority, whether to promote priority for sa.
     * @param isVisible, whether show window when start fail by afterCheckExecuter.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCallWithErrMsg(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId, std::string &errMsg,
        bool isSilent = false, bool promotePriority = false, bool isVisible = false) override;

    /**
     * Start Ability for prelauch.
     *
     * @param want, Special want for service type's ability.
     * @param frameNum, Special frameNum for remove start window num.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityForPrelaunch(const Want &want, const int32_t frameNum) override;

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

    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);
    void OnCallConnectDied(std::shared_ptr<CallRecord> callRecord);
    void HandleLoadTimeOut(int64_t abilityRecordId, bool isHalf = false, bool isExtension = false);
    void HandleActiveTimeOut(int64_t abilityRecordId);
    void HandleInactiveTimeOut(int64_t abilityRecordId);
    void HandleForegroundTimeOut(int64_t abilityRecordId, bool isHalf = false, bool isExtension = false);
    void HandleConnectTimeOut(int64_t abilityRecordId, bool isHalf = false);
    void HandleShareDataTimeOut(int64_t uniqueId);
    int32_t GetShareDataPairAndReturnData(std::shared_ptr<AbilityRecord> abilityRecord,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam);

    int32_t StartAbilityByFreeInstall(const StartAbilityWrapParam &param);

    int StartAbilityWrap(const StartAbilityWrapParam &startAbilityWrapParam);

    int StartAbilityInner(StartAbilityWrapParam &param);

    int32_t StartExtensionAbilityInner(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId,
        AppExecFwk::ExtensionAbilityType extensionType,
        bool checkSystemCaller = true,
        bool isImplicit = false,
        bool isDlp = false,
        bool isStartAsCaller = false);

    int RequestModalUIExtensionInner(Want want);

    int PreloadUIExtensionAbilityInner(const Want &want, std::string &bundleName,
        int32_t userId = DEFAULT_INVAL_VALUE, int32_t hostPid = DEFAULT_INVAL_VALUE,
        int32_t requestCode = DEFAULT_INVAL_VALUE);

    int StartAbilityForOptionWrap(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, bool isPendingWantCaller, int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE, bool isStartAsCaller = false, uint32_t callerTokenId = 0,
        bool isImplicit = false, bool isCallByShortcut = false);

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
        bool isImplicit = false,
        bool isAppCloneSelector = false,
        uint32_t callerAccessTokenId = 0);

    int ImplicitStartAbilityAsCaller(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * @brief called when the module's onAcceptWant done to notify ability mgr to continue
     * @param want request param being accepted
     * @param flag specified flag return by application
     * @param requestId a number represents a request
     * @param userId The user id
     */
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId, int32_t userId);

    /**
     * @brief called when the module's onAcceptWant happens time out
     * @param requestId a number represents a request
     * @param userId The user id
     */
    void OnStartSpecifiedAbilityTimeoutResponse(int32_t requestId, int32_t userId);

    /**
     * @brief called when the module's onNewProcessRequest returns a flag
     * @param flag process flag
     * @param userId The user id
     * @param requestId a number represents a request
     */
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t userId, int32_t requestId = 0,
        const std::string &callerProcessName = "", int32_t recordId = 0);

    /**
     * @brief called when the module's onNewProcessRequest happens time out
     * @param requestId a number represents a request
     * @param userId The user id
     */
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId, int32_t userId);
    /**
     * @brief called when the specified request fail fast
     * @param requestId a number represents a request
     * @param userId The user id
     */
    void OnStartSpecifiedFailed(int32_t requestId, int32_t userId);

    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override;
    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override;
    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override;
    virtual int GetAllIntentExemptionInfo(std::vector<AppExecFwk::IntentExemptionInfo> &info) override;
    int GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId);
    void GetAbilityRunningInfo(std::vector<AbilityRunningInfo> &info, std::shared_ptr<AbilityRecord> abilityRecord);
    void GetExtensionRunningInfo(std::shared_ptr<BaseExtensionRecord> &abilityRecord, const int32_t userId,
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

    virtual int StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback,
        bool isAppRecovery = false) override;

    virtual int StopUser(int userId, const sptr<IUserCallback> &callback) override;

    virtual int LogoutUser(int32_t userId, sptr<IUserCallback> callback) override;

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

    sptr<IWindowManagerServiceHandler> GetWMSHandler();

    virtual int PrepareTerminateAbility(const sptr<IRemoteObject> &token,
        sptr<IPrepareTerminateCallback> &callback) override;

    /**
     * PrepareTerminateAbilityDone, called when PrepareTerminateAbility call is done.
     *
     * @param token, the token of the ability to terminate.
     * @param callback callback.
     */
    virtual void PrepareTerminateAbilityDone(const sptr<IRemoteObject> &token, bool isTerminate) override;

    /**
     * KillProcessWithPrepareTerminateDone, called when KillProcessWithPrepareTerminate call is done.
     *
     * @param moduleName, the module name of the application.
     * @param prepareTermination, the result of prepareTermination call of the module.
     * @param isExist, whether the prepareTerminate functions are implemented.
     */
    virtual void KillProcessWithPrepareTerminateDone(const std::string &moduleName,
        int32_t prepareTermination, bool isExist) override;

    void HandleFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo);

    void HandleUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo);

    void HandleWindowVisibilityChanged(
        const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos);

    virtual int GetDialogSessionInfo(const std::string &dialogSessionId,
        sptr<DialogSessionInfo> &dialogSessionInfo) override;

    virtual int SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllowed) override;

    int CreateCloneSelectorDialog(AbilityRequest &request, int32_t userId, const std::string &replaceWantString = "");

    void RemoveSelectorIdentity(int32_t tokenId);

    virtual int RegisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer,
        const std::string &bundleName) override;

    virtual int UnregisterAbilityFirstFrameStateObserver(
        const sptr<IAbilityFirstFrameStateObserver> &observer) override;

    bool GetAnimationFlag();

#endif

    void ClearUserData(int32_t userId);

    /**
     * notify callers disconnect abilities before clearUserData
     * or can not find connectManager_ by userId when onAbilityDied.
     *
     * @param userId userId.
     */
    void DisconnectBeforeCleanupByUserId(int32_t userId);

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

    virtual int CheckUIExtensionIsFocused(
        uint32_t uiExtensionTokenId, bool& isFocused, uint64_t displayId = 0) override;

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

    virtual int32_t RegisterForegroundAppObserver(sptr<AbilityRuntime::IForegroundAppConnection> observer) override;
    virtual int32_t UnregisterForegroundAppObserver(sptr<AbilityRuntime::IForegroundAppConnection> observer) override;

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
     * @param uid.
     */
    void AppUpgradeCompleted(int32_t uid, int32_t installType);

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

    /**
     * Record the exit reason of a killed process.
     * @param pid The process id.
     * @param uid The process uid.
     * @param exitReason The reason of process exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RecordProcessExitReason(int32_t pid, int32_t uid, const ExitReason &exitReason) override;

    /**
     * Force app exit and record exit reason.
     * @param pid The process id.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t KillAppWithReason(const int32_t pid, const ExitReasonCompability &ExitReason) override;

    /**
     * Force bundle exit and record exit reason.
     * @param bundleName Bundle name of kill app.
     * @param appIndex The app index of app clone.
     * @param userId User ID.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t KillBundleWithReason(const std::string &bundleName, int32_t userId, int32_t appIndex,
        const ExitReasonCompability &exitReason) override;

    /**
     * Record app exit reason.
     * @param pid The process id.
     * @param uid The process uid.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RecordAppWithReason(
        const int32_t pid, const int32_t uid, const ExitReasonCompability &exitReason) override;

    int32_t GetConfiguration(AppExecFwk::Configuration& config);

    int32_t GetConfiguration(AppExecFwk::Configuration& config, int32_t userId);
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
     * @param params start parameters.
     * @param isColdStart the session of the ability is or not cold start.
     */
    virtual void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo,
        AbilityRuntime::StartParamsBySCB &params, bool &isColdStart) override;

    /**
     * Start specified ability by SCB.
     *
     * @param want Want information.
     * @param params The parameters to start specified ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartSpecifiedAbilityBySCB(const Want &want, const StartSpecifiedAbilityParams &params) override;

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
     * @brief get ability manager collaborator.
     * @return Returns object pointer on success, others on null.
     */
    virtual sptr<IAbilityManagerCollaborator> GetAbilityManagerCollaborator() override;

    /**
     * @brief Get collaborator.
     * @param type collaborator type.
     * @return nullptr or IAbilityManagerCollaborator stpr.
    */
    sptr<IAbilityManagerCollaborator> GetCollaborator(int32_t type);

    virtual int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate) override;

    virtual int32_t KillProcessWithPrepareTerminate(const std::vector<int32_t> &pids, bool clear) override;

    /**
     * kill the process with reason
     *
     * @param pid id of process.
     * @param  reason, kill process reason.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t KillProcessWithReason(int32_t pid, const ExitReason &reason) override;

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
     * @brief Retrieves the auto startup status of the current application.
     * @param isAutoStartEnabled Indicates whether auto startup is enabled for the current application.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAutoStartupStatusForSelf(bool &isAutoStartEnabled) override;

    /**
     * @brief Manual start auto startup apps, EDM use only.
     * @param userId Indicates which user's auto startup apps to be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ManualStartAutoStartupApps(int32_t userId) override;

    /**
     * @brief Query the caller's Token ID for anco.
     * @param userId Indicates the user ID.
     * @param asCallerForAncoSessionId Indicates the anco session Id of cached information.
     * @param callerTokenId Indicates the output caller Token ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode QueryCallerTokenIdForAnco(int32_t userId, const std::string &asCallerForAncoSessionId,
        uint32_t &callerTokenId) override;

    /**
     * PrepareTerminateAbilityBySCB, prepare to terminate ability by scb.
     *
     * @param sessionInfo the session info of the ability to start.
     * @param isTerminate the result of ability onPrepareToTerminate.
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
    int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override;

    /**
     * @brief Detach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DetachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override;

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
        const InsightIntentExecuteParam &param, int32_t userId = DEFAULT_INVAL_VALUE);

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

    void RemoveLauncherDeathRecipient(int32_t userId);
    /**
     * @brief Set application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether the application is prohibited from changing the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override;

    /**
     * @brief Cancel application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether the application is prohibited from changing the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override;

    /**
     * @brief Get foreground ui abilities.
     * @param list Foreground ui abilities.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list) override;

    /**
     * @brief Update session info.
     * @param sessionInfos The vector of session info.
     */
    virtual int32_t UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, int32_t userId,
        std::vector<int32_t> &sessionIds) override;

    /**
     * @brief Restart app self.
     * @param want The ability type must be UIAbility.
     * @param isAppRecovery True indicates that the app is restarted because of recovery.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RestartApp(const AAFwk::Want &want, bool isAppRecovery = false) override;

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
        const std::string& reason, int32_t userId);

    void HandleRestartResidentProcessDependedOnWeb();

    int32_t TerminateMission(int32_t missionId) override;

    int32_t UpdateAssociateConfigList(const std::map<std::string, std::list<std::string>>& configs,
        const std::list<std::string>& exportConfigs, int32_t flag) override;

    int32_t BlockAllAppStart(bool flag) override;

    int SetWantForSessionInfo(sptr<SessionInfo> sessionInfo);

    int32_t StartUIAbilityBySCBDefaultCommon(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        AbilityRuntime::StartParamsBySCB &params, bool &isColdStart);

    int32_t NotifySCBToRecoveryAfterInterception(const AbilityRequest &abilityRequest);

    /**
     * Judge if Caller-Application is in background state.
     *
     * @param abilityRequest, abilityRequest.
     * @param isBackgroundCall, Indicates the Caller-Application state.
     *                          TRUE: The Caller-Application is not in focus and not in foreground state.
     *                          FALSE: The Caller-Application is in focus or in foreground state.
     * @param isSelector Indicates the scenario of an application selection box. When set to true,
     * the specifiedTokenId is used to verify permissions.
     * @param specifyTokenId The origin accessTokenId.
     * @param isData Is DataAbility.
     * @return Returns ERR_OK on check success, others on check failure.
     */
    int IsCallFromBackground(const AbilityRequest &abilityRequest, bool &isBackgroundCall, bool isSelector,
        uint32_t specifyTokenId = 0, bool isData = false);

    void EnableListForSCBRecovery(int32_t userId) const;

    int32_t UpdateKeepAliveEnableState(const std::string &bundleName, const std::string &moduleName,
        const std::string &mainElement, bool updateEnable, int32_t userId);

    bool IsInStatusBar(uint32_t accessTokenId, int32_t uid, bool isMultiInstance);

    bool IsSupportStatusBar(int32_t uid);

    bool IsSceneBoardReady(int32_t userId);

    /**
     * Set keep-alive flag for application under a specific user.
     * @param bundleName Bundle name.
     * @param userId User Id.
     * @param flag Keep-alive flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetApplicationKeepAlive(const std::string &bundleName, int32_t userId, bool flag) override;

    /**
     * Get keep-alive applications by EDM.
     * @param appType Application type.
     * @param userId User Id.
     * @param list List of Keep-alive information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t QueryKeepAliveApplications(int32_t appType, int32_t userId,
        std::vector<KeepAliveInfo> &list) override;

    /**
     * Set keep-alive flag for application under a specific user by EDM.
     * @param bundleName Bundle name.
     * @param userId User Id.
     * @param flag Keep-alive flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetApplicationKeepAliveByEDM(const std::string &bundleName, int32_t userId,
        bool flag, bool isAllowUserToCancel = false) override;

    /**
     * Get keep-alive applications by EDM.
     * @param appType Application type.
     * @param userId User Id.
     * @param list List of Keep-alive information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t QueryKeepAliveApplicationsByEDM(int32_t appType, int32_t userId,
        std::vector<KeepAliveInfo> &list) override;

    /**
     * Add query ERMS observer.
     *
     * @param callerToken, The caller ability token.
     * @param observer, The observer of the ability to query ERMS.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t AddQueryERMSObserver(sptr<IRemoteObject> callerToken,
        sptr<AbilityRuntime::IQueryERMSObserver> observer) override;

    /**
     * Query atomic service ERMS rule.
     *
     * @param callerToken, The caller ability token.
     * @param appId, The appId of the atomic service.
     * @param startTime, The startTime of the query.
     * @param rule, The returned ERMS rule.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t QueryAtomicServiceStartupRule(sptr<IRemoteObject> callerToken,
        const std::string &appId, const std::string &startTime, AtomicServiceStartupRule &rule) override;

    /**
     * Restart atomic service.
     *
     * @param callerToken, The caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RestartSelfAtomicService(sptr<IRemoteObject> callerToken) override;

    int StartUIAbilityForOptionWrap(const Want &want, const StartOptions &options, sptr<IRemoteObject> callerToken,
        bool isPendingWantCaller, int32_t userId, int requestCode, uint32_t callerTokenId = 0, bool isImplicit = false,
        bool isCallByShortcut = false);

    /**
     * KillProcessForPermissionUpdate, call KillProcessForPermissionUpdate() through proxy object,
     * force kill the application by accessTokenId, notify exception to SCB.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillProcessForPermissionUpdate(uint32_t accessTokenId) override;

    std::shared_ptr<AppExitReasonHelper> GetAppExitReasonHelper()
    {
        return appExitReasonHelper_;
    }

    /**
     * Register hidden start observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterHiddenStartObserver(const sptr<IHiddenStartObserver> &observer) override;

    /**
     * Unregister hidden start observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterHiddenStartObserver(const sptr<IHiddenStartObserver> &observer) override;

    /**
     * Query preload uiextension record.
     *
     * @param element, The uiextension ElementName.
     * @param moduleName, The uiextension moduleName.
     * @param hostBundleName, The uiextension caller hostBundleName.
     * @param recordNum, The returned count of uiextension.
     * @param userId, The User Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t QueryPreLoadUIExtensionRecord(const AppExecFwk::ElementName &element,
                                                  const std::string &moduleName,
                                                  const int32_t hostPid,
                                                  int32_t &recordNum,
                                                  int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Revoke delegator.
     *
     * @param token, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RevokeDelegator(sptr<IRemoteObject> token) override;

    virtual int32_t SetOnNewWantSkipScenarios(sptr<IRemoteObject> callerToken, int32_t scenarios) override;

    /**
     * Get all insight intent infos.
     * @param flag, the get type.
     * @param infos, the insight intent infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAllInsightIntentInfo(
        AbilityRuntime::GetInsightIntentFlag flag,
        std::vector<InsightIntentInfoForQuery> &infos,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Get specified bundleName insight intent infos.
     * @param flag, the get type.
     * @param infos, the insight intent infos.
     * @param bundleName, The get insightIntent bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetInsightIntentInfoByBundleName(
        AbilityRuntime::GetInsightIntentFlag flag,
        const std::string &bundleName,
        std::vector<InsightIntentInfoForQuery> &infos,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Get specified intentName insight intent infos.
     * @param flag, the get type.
     * @param infos, the insight intent infos.
     * @param bundleName, The get insightIntent bundleName.
     * @param moduleName, The get insightIntent moduleName.
     * @param intentName, The get intent name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetInsightIntentInfoByIntentName(
        AbilityRuntime::GetInsightIntentFlag flag,
        const std::string &bundleName,
        const std::string &moduleName,
        const std::string &intentName,
        InsightIntentInfoForQuery &info,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

   /**
     * StartAbilityWithWait, send want and abilityStartWithWaitObserver to abms.
     *
     * @param want Ability want.
     * @param observer ability foreground notify observer for aa tool.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartAbilityWithWait(Want &want, sptr<IAbilityStartWithWaitObserver> &observer) override;

    int32_t UpdateKioskApplicationList(const std::vector<std::string> &appList) override;

    int32_t EnterKioskMode(sptr<IRemoteObject> callerToken) override;

    int32_t ExitKioskMode(sptr<IRemoteObject> callerToken) override;

    int32_t GetKioskStatus(AAFwk::KioskStatus &kioskStatus) override;

    std::shared_ptr<AbilityInterceptorExecuter> GetAbilityInterceptorExecuter();

     /**
     * Set keep-alive flag for app service extension under u1 user.
     * @param bundleName Bundle name.
     * @param flag Keep-alive flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetAppServiceExtensionKeepAlive(const std::string &bundleName, bool flag) override;

    /**
     * Get keep-alive app service extensions.
     * @param list List of Keep-alive information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t QueryKeepAliveAppServiceExtensions(std::vector<KeepAliveInfo> &list) override;

    /**
     * Register sa interceptor.
     * @param interceptor, The sa interceptor.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterSAInterceptor(sptr<AbilityRuntime::ISAInterceptor> interceptor) override;

    virtual int32_t NotifyStartupExceptionBySCB(int32_t requestId) override;

    /**
     * Check if the app is restart-limited.
     * @return Returns true on being limited.
     */
    bool IsRestartAppLimit() override;
    int32_t SignRestartProcess(int32_t pid, int32_t userId);

    /**
     * Preload application.
     * @param bundleName Name of the application.
     * @param userId user id.
     * @param appIndex app clone index.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t PreloadApplication(const std::string &bundleName, int32_t userId, int32_t appIndex) override;

    int StartAbilityWithRemoveIntentFlag(const StartAbilityWrapParam &param);
    
    // MSG 0 - 20 represents timeout message
    static constexpr uint32_t LOAD_TIMEOUT_MSG = 0;
    static constexpr uint32_t ACTIVE_TIMEOUT_MSG = 1;
    static constexpr uint32_t INACTIVE_TIMEOUT_MSG = 2;
    static constexpr uint32_t TERMINATE_TIMEOUT_MSG = 4;
    static constexpr uint32_t FOREGROUND_TIMEOUT_MSG = 5;
    static constexpr uint32_t BACKGROUND_TIMEOUT_MSG = 6;
    static constexpr uint32_t SHAREDATA_TIMEOUT_MSG = 7;
    static constexpr uint32_t LOAD_HALF_TIMEOUT_MSG = 8;
    static constexpr uint32_t FOREGROUND_HALF_TIMEOUT_MSG = 9;
    static constexpr uint32_t CONNECT_TIMEOUT_MSG = 10;
    static constexpr uint32_t CONNECT_HALF_TIMEOUT_MSG = 11;

    static constexpr uint32_t MIN_DUMP_ARGUMENT_NUM = 2;
    static constexpr uint32_t MAX_WAIT_SYSTEM_UI_NUM = 600;
    static constexpr uint32_t MAX_WAIT_SETTINGS_DATA_NUM = 300;

    enum {
        ABILITY_MOVE_TO_FOREGROUND_CODE = 0,
        ABILITY_MOVE_TO_BACKGROUND_CODE,
        TERMINATE_ABILITY_CODE
    };

protected:
    /**
     * AbilityMgr's request is done.
     *
     * @param token Ability token.
     * @param state Application state.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) override;
    int GetUidByBundleName(std::string bundleName);

    /**
     * Application state changed callback.
     * Only observe APP_STATE_FOREGROUND and APP_STATE_BACKGROUND
     *
     * @param info Application state data.
     */
    void OnAppStateChanged(const AppInfo &info) override;

    void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) override;

    void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
        pid_t diedPid = AppExecFwk::INVALID_DIED_PID) override;

    /**
     * @brief Notify abilityms app process pre cache
     * @param pid process pid.
     * @param userId userId Designation User ID.
     */
    void NotifyAppPreCache(int32_t pid, int32_t userId) override;

    /**
     * @brief Notify abilityms app process OnRemoteDied
     * @param abilityTokens abilities in died process.
     */
    void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) override;

    /**
     * @brief Notify abilityms start process failed when load ability
     * @param abilityTokens abilities in died process.
     */
    void OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens) override;

    /**
     * @brief Notify abilityms process info when an app dies
     * @param accessTokenId app accessTokenId.
     * @param exitInfo process running info.
     * @param bundleName app bundleName.
     * @param abilityNames started abilities.
     * @param uiExtensionNames started ui extensions.
     */
    void OnCacheExitInfo(uint32_t accessTokenId, const AppExecFwk::RunningProcessInfo &exitInfo,
        const std::string &bundleName, const std::vector<std::string> &abilityNames,
        const std::vector<std::string> &uiExtensionNames) override;

    /**
     * @brief Record the signal reason when an application process exits.
     *
     * @param pid Process ID of the exited application process.
     * @param uid User ID of the exited application process.
     * @param signal Signal number that caused the process to exit.
     * @param bundleName Bundle name of the exited application.
     */
    void RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName) override;

    int32_t GetCollaboratorType(const std::string &codePath) const;

    /**
     * @brief kill process for Collaborator
     * @param collaboratorType collaborator type.
     * @param bundleName destination bundleName.
     * @param userId process user id.
     */
    int32_t KillProcessForCollaborator(int32_t collaboratorType, const std::string &bundleName, int32_t userId);
    
    /**
     * Check if Caller is allowed to start AppServiceExtension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @param isVerifyAppIdentifierAllowList, isVerifyAppIdentifierAllowList
     * @param isFromConnect, isFromConnect
     * @return Returns whether the caller is allowed to start AppServiceExtension.
     */
    int32_t CheckCallAppServiceExtensionPermission(const AbilityRequest &abilityRequest,
        std::shared_ptr<BaseExtensionRecord> targetService, bool isFromConnect);

    static void HandleAutoStartupReadyCallback(const char *key, const char *value, void *context);

    /**
     * UnPreload UIExtension with want, send want to ability manager service.
     *
     * @param extensionAbilityId The extension ability Id.
     * @param userId The User Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ClearPreloadedUIExtensionAbility(
        int32_t extensionAbilityId, int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * clear all Preload UIExtension with want, send want to ability manager service.
     *
     * @param userId The User Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ClearPreloadedUIExtensionAbilities(int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * @brief Register preload ui extension host client.
     * @param callerToken Caller ability token.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief UnRegister preload ui extension host client.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnRegisterPreloadUIExtensionHostClient(int32_t callerPid = DEFAULT_INVAL_VALUE) override;

    /**
     * @brief Get list of applications launched before the first unlock.
     * @param userId The User Id.
     * @param userLockedBundleList List of applications launched before the first unlock.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetUserLockedBundleList(int32_t userId,
        std::unordered_set<std::string> &userLockedBundleList) override;

private:
    int GetTopAbilityInner(sptr<IRemoteObject> &token, uint64_t displayId = 0);
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
#ifndef DISABLE_LAUNCHER
    /**
     * start highest priority ability.
     *
     */
    int StartHighestPriorityAbility(int32_t userId, uint64_t displayId, bool isBoot, bool isAppRecovery = false);
#endif
    /**
     * connect bms.
     *
     */
    void ConnectServices();

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

    int StartRemoteAbility(const Want &want, int requestCode, int32_t validUserId,
        const sptr<IRemoteObject> &callerToken, uint32_t specifyTokenId = 0);
    int StartUIAbilityBySCBDefault(sptr<SessionInfo> sessionInfo, AbilityRuntime::StartParamsBySCB &params,
        bool &isColdStart);
    int StartUIAbilityByPreInstallInner(sptr<SessionInfo> sessionInfo, uint32_t specifyTokenId,
        AbilityRuntime::StartParamsBySCB &params, bool &isColdStart);
    int32_t PreStartInner(const FreeInstallInfo& taskInfo);
    void RemovePreStartSession(const std::string& sessionId);

    int32_t ConnectLocalAbility(
        const Want &want,
        const int32_t userId,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ExtensionAbilityType extensionType,
        const sptr<SessionInfo> &sessionInfo = nullptr,
        bool isQueryExtensionOnly = false,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr,
        uint64_t specifiedFullTokenId = 0,
        int32_t loadTimeout = 0);

    int DisconnectLocalAbility(const sptr<IAbilityConnection> &connect);
    int32_t HandleExtensionConnectionByUserId(sptr<IAbilityConnection> connect, int32_t userId,
        std::function<int32_t(std::shared_ptr<AbilityConnectManager>, sptr<IAbilityConnection>)> func);
    int ConnectRemoteAbility(Want &want, const sptr<IRemoteObject> &callerToken, const sptr<IRemoteObject> &connect);
    int DisconnectRemoteAbility(const sptr<IRemoteObject> &connect);
    int PreLoadAppDataAbilities(const std::string &bundleName, const int32_t userId);
    void PreLoadAppDataAbilitiesTask(const std::string &bundleName, const int32_t userId);
    int StartAbilityPublicPrechainCheck(StartAbilityParams &params);
    int StartAbilityPrechainInterceptor(StartAbilityParams &params);
    bool StartAbilityInChain(StartAbilityParams &params, int &result);
    void InitWindowVisibilityChangedListener();
    void FreeWindowVisibilityChangedListener();
    bool CheckProcessIsBackground(int32_t pid, AbilityState currentState);

    bool CheckIfOperateRemote(const Want &want);
    std::string AnonymizeDeviceId(const std::string& deviceId);
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
    void ForceTerminateServiceExtensionByPid(int32_t pid, int32_t userId);

    void DumpInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionInner(const std::string &args, std::vector<std::string> &info);
    void DumpStateInner(const std::string &args, std::vector<std::string> &info);
    void DataDumpStateInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionListInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionInfosInner(const std::string &args, std::vector<std::string> &info);

    bool JudgeMultiUserConcurrency(int32_t userId);
    bool CheckCrossUser(const int32_t userId, AppExecFwk::ExtensionAbilityType extensionType);
    void SendExtensionReport(EventInfo &eventInfo, int32_t errCode, bool isService = false);
    void SendIntentReport(EventInfo &eventInfo, int32_t errCode, const std::string &intentName);
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
    int Dump(const std::vector<std::u16string>& args, std::string& result);

    // multi user
    void StartFreezingScreen();
    void StopFreezingScreen();
    void UserStarted(int32_t userId);
    int SwitchToUser(int32_t oldUserId, int32_t userId, uint64_t displayId, sptr<IUserCallback> callback,
        bool isAppRecovery = false);
    void SwitchManagers(int32_t userId, bool switchUser = true);
    bool StartUserApps(int32_t oldUserId);
    void PauseOldUser(int32_t userId);
    void PauseOldMissionListManager(int32_t userId);
    void PauseOldConnectManager(int32_t userId);
    bool IsSystemUI(const std::string &bundleName) const;
    int32_t GetUidByCloneBundleInfo(std::string &bundleName, int32_t callerUid, int32_t userId,
        int32_t &appIndex) const;
    sptr<IWantSender> GetWantSenderByUserId(const WantSenderInfo &wantSenderInfo,
        const sptr<IRemoteObject> &callerToken, int32_t uid, int32_t callerUid, int32_t callerUserId);

    bool VerificationAllToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<DataAbilityManager> GetCurrentDataAbilityManager();
    std::shared_ptr<DataAbilityManager> GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token);
    int32_t HandleExtensionAbility(sptr<IAbilityConnection> connect,
        std::function<int32_t(std::shared_ptr<AbilityConnectManager>, sptr<IAbilityConnection>)>);
    std::vector<std::shared_ptr<AbilityConnectManager>> GetConnectManagers();

    int32_t GetServiceMapByUserId(AbilityConnectManager::ServiceMapType &serviceMap, int32_t userId);
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByUserId(
        int32_t userId, const AppExecFwk::ExtensionAbilityType type);
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByToken(
        const sptr<IRemoteObject> &token, const AppExecFwk::ExtensionAbilityType type);

    std::unordered_map<int, std::shared_ptr<UIExtensionAbilityManager>> GetUIExtensionAbilityManagers();
    std::shared_ptr<UIExtensionAbilityManager> GetCurrentUIExtensionAbilityManager();
    std::shared_ptr<UIExtensionAbilityManager> GetUIExtensionAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<UIExtensionAbilityManager> GetUIExtensionAbilityManagerByToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<UIExtensionAbilityManager> GetUIExtensionAbilityManagerByAbilityRecordId(
        const int64_t &abilityRecordId);

    std::unordered_map<int, std::shared_ptr<CommonExtensionManager>> GetCommonExtensionManagers();
    std::shared_ptr<CommonExtensionManager> GetCurrentCommonExtensionManager();
    std::shared_ptr<CommonExtensionManager> GetCommonExtensionManagerByUserId(int32_t userId);
    std::shared_ptr<CommonExtensionManager> GetCommonExtensionManagerByToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<CommonExtensionManager> GetCommonExtensionManagerByAbilityRecordId(const int64_t &abilityRecordId);

    std::shared_ptr<PendingWantManager> GetCurrentPendingWantManager();
    std::shared_ptr<PendingWantManager> GetPendingWantManagerByUserId(int32_t userId);
    std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> GetMissionListManagers();
    std::shared_ptr<MissionListManagerInterface> GetCurrentMissionListManager();
    std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> GetUIAbilityManagers();
    std::shared_ptr<UIAbilityLifecycleManager> GetCurrentUIAbilityManager();
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUserId(int32_t userId) const;
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUid(int32_t uid);
    bool JudgeSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord);
    bool IsAppSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord);

    int32_t GetValidUserId(int32_t userId);

    int DelegatorMoveMissionToFront(int32_t missionId);

    bool IsNeedTimeoutForTest(const std::string &abilityName, const std::string &state) const;

    void StartResidentApps(int32_t userId);

    void StartKeepAliveApps(int32_t userId);

    void StartAutoStartupApps(int32_t userId, bool isManualStart = false);
    void StartAutoStartupApps(std::queue<AutoStartupInfo> infoList, int32_t userId);
    void SubscribeUserUnlockedEvent();
    void SubscribeScreenUnlockedEvent();
    std::function<void(int32_t)> GetScreenUnlockCallback();
    std::function<void(int32_t)> GetUserScreenUnlockCallback();
    void UnSubscribeScreenUnlockedEvent();
    void RetrySubscribeUnlockedEvent(int32_t retryCount, std::shared_ptr<EventFwk::CommonEventSubscriber> subscriber,
        bool isUserUnlockSubscriber = false);
    void RemoveScreenUnlockInterceptor();
    void AddWatchParameters();

    void RemoveUnauthorizedLaunchReasonMessage(const Want &want, AbilityRequest &abilityRequest,
        uint32_t callerTokenId);

    int VerifyAccountPermission(int32_t userId);

    int CheckStaticCfgPermissionForAbility(const AppExecFwk::AbilityInfo &abilityInfo, uint32_t tokenId);

    int CheckStaticCfgPermissionForSkill(const AppExecFwk::AbilityRequest &abilityRequest, uint32_t tokenId);

    bool CheckOneSkillPermission(const AppExecFwk::Skill &skill, uint32_t tokenId);

    int CheckStaticCfgPermission(const AppExecFwk::AbilityRequest &abilityRequest, bool isStartAsCaller,
        uint32_t callerTokenId, bool isData = false, bool isSaCall = false, bool isImplicit = false);

    int CheckPermissionForUIService(AppExecFwk::ExtensionAbilityType extensionType,
        const Want &want, const AbilityRequest &abilityRequest);

    bool CheckPermissionForKillCollaborator();

    bool GetValidDataAbilityUri(const std::string &abilityInfoUri, std::string &adjustUri);

    int GenerateExtensionAbilityRequest(const Want &want, AbilityRequest &request,
        const sptr<IRemoteObject> &callerToken, int32_t userId, const std::string hostBundleName = "");
    std::string GetHostBundleName(const std::string hostBundleName,
        const sptr<IRemoteObject> &callerToken);
    int32_t InitialAbilityRequest(AbilityRequest &request, const StartAbilityInfo &abilityInfo) const;
    int CheckOptExtensionAbility(const Want &want, AbilityRequest &abilityRequest,
        int32_t validUserId, AppExecFwk::ExtensionAbilityType extensionType, bool isImplicit = false,
        bool isStartAsCaller = false);

    void SubscribeBackgroundTask();

    void UnSubscribeBackgroundTask();

    void SubscribeBundleEventCallback();

    void UnsubscribeBundleEventCallback();

    void ReportAbilityStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);

    void ReportAbilityAssociatedStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo, int64_t type,
        const sptr<IRemoteObject> &callerToken);

    void ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo, sptr<IRemoteObject> callerToken);

    void ReportAppConnectOtherExtensionEvent(const AppExecFwk::AbilityInfo &abilityInfo, const Want &want);

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

#ifdef SUPPORT_AUTO_FILL
    /**
     * Check if Caller is allowed to start AutoFillExtension.
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start ServiceExtension.
     */
    int CheckCallAutoFillExtensionPermission(const AbilityRequest &abilityRequest);
#endif // SUPPORT_AUTO_FILL
    /**
     * Check if Caller is allowed to start other Extension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @param validUserId, Valid user id.
     * @return Returns whether the caller is allowed to start OtherExtension.
     */
    int CheckCallOtherExtensionPermission(const AbilityRequest &abilityRequest, int32_t validUserId = INVALID_USER_ID);

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
     * @param isSelector Indicates the scenario of an application selection box.
     * @param specifyTokenId The origin accessTokenId.
     * @param isCallByShortcut The start reason.
     * @return Returns whether the caller is allowed to start Ability.
     */
    int CheckCallAbilityPermission(const AbilityRequest &abilityRequest, bool isSelector, uint32_t specifyTokenId = 0,
        bool isCallByShortcut = false, bool isFreeInstallFromService = false);

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
     * @param validUserId, Valid user id.
     * @return Returns ERR_OK when allowed, others when check failed.
     */
    int CheckUIExtensionPermission(const AbilityRequest &abilityRequest, int32_t validUserId);

    /**
     * @brief Check CallerInfoQueryExtension permission
     *
     * @param abilityRequest The ability request.
     * @return Returns ERR_OK when allowed, others when check failed.
     */
    int CheckCallerInfoQueryExtensionPermission(const AbilityRequest &abilityRequest);

    int CheckFileAccessExtensionPermission(const AbilityRequest &abilityRequest);

    bool IsTargetPermission(const Want &want) const;

    bool IsDelegatorCall(const AppExecFwk::RunningProcessInfo &processInfo, const AbilityRequest &abilityRequest) const;

    bool CheckNewRuleSwitchState(const std::string &param);

    void UpdateFocusState(std::vector<AbilityRunningInfo> &info);

    AAFwk::PermissionVerification::VerificationInfo CreateVerificationInfo(
        const AbilityRequest &abilityRequest, bool isData = false, bool isShell = false, bool isSA = false);

    int AddStartControlParam(Want &want, const sptr<IRemoteObject> &callerToken);

    AAFwk::EventInfo BuildEventInfo(const Want &want, int32_t userId);

    ErrCode IsUIAbilityAlreadyExist(const Want &want, const std::string &specifiedFlag,
        int32_t appIndex, const std::string &instanceKey, AppExecFwk::LaunchMode launchMode);
    
    bool IsAppCloneOrMultiInstance(const Want &want, const std::shared_ptr<AbilityRecord> callerRecord,
        int32_t &appIndex, const std::string &callerInstanceKey);

#ifdef WITH_DLP
    int CheckDlpForExtension(
        const Want &want, const sptr<IRemoteObject> &callerToken,
        int32_t userId, AAFwk::EventInfo &eventInfo, const EventName &eventName);
#endif // WITH_DLP

    void InitStartupFlag();

    bool IsCrossUserCall(int32_t userId) const;

    int32_t RequestDialogServiceInner(const Want &want, const sptr<IRemoteObject> &callerToken,
        int requestCode, int32_t userId);

    bool CheckCallingTokenId(const std::string &bundleName, int32_t userId = INVALID_USER_ID, int32_t appIndex = 0);
    bool IsCallerSceneBoard();

    void ReleaseAbilityTokenMap(const sptr<IRemoteObject> &token);

    bool CheckCollaboratorType(int32_t type);

    bool CheckUserIdActive(int32_t userId);

    void GetConnectManagerAndUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo,
        std::shared_ptr<UIExtensionAbilityManager> &connectManager, std::shared_ptr<BaseExtensionRecord> &targetAbility,
        bool needCheck = false);

    virtual int RegisterSessionHandler(const sptr<IRemoteObject> &object) override;
    int32_t CheckWantForSplitMode(const AAFwk::Want &secondaryWant, sptr<IRemoteObject> callerToken,
        int32_t validUserId, int32_t appIndex);
    int32_t StartUIAbilitiesInSplitWindowModeHandleWant(const AAFwk::Want &secondaryWant,
        sptr<IRemoteObject> callerToken, AbilityRequest &abilityRequest);
    int32_t GenerateAbilityForSplitMode(const AAFwk::Want &secondaryWant, AbilityRequest &abilityRequest,
        sptr<IRemoteObject> callerToken, int32_t requestCode, int32_t specifyTokenId, int32_t appIndex,
        int32_t validUserId);
    int32_t StartUIAbilitiesHandleWant(const Want &want, sptr<IRemoteObject> callerToken,
        std::vector<AbilityRequest> &abilityRequestList);
    int32_t StartUIAbilitiesCheckDlp(const Want &want, sptr<IRemoteObject> callerToken, int32_t userId);
    int32_t StartUIAbilitiesInterceptorCheck(const Want &want, AbilityRequest &abilityRequest,
        sptr<IRemoteObject> callerToken, int32_t appIndex, int32_t userId);
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

    /**
     * judge system params for picker
     * @param WantParams, The wantparams of the want.
     */
    bool JudgeSystemParamsForPicker(const WantParams &parameters);

    void SetPickerElementNameAndParams(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId);

    void SetAutoFillElementName(const sptr<SessionInfo> &extensionSessionInfo);

    /**
     * @brief Start extension ability with insight intent
     * @param want, the want of the ability to start.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartExtensionAbilityWithInsightIntent(const Want &want,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED,
        int32_t userId = DEFAULT_INVAL_VALUE);

    bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord,
        const int32_t oriValidUserId);

    void InitInterceptor();
    void InitInterceptorForScreenUnlock();
    void InitPushTask();
    void InitAppSpawnMsgPipe();
    void InitDeepLinkReserve();

    bool CheckSenderWantInfo(int32_t callerUid, const WantSenderInfo &wantSenderInfo);

    int32_t UninstallAppInner(const std::string &bundleName, const int32_t uid, int32_t appIndex, const bool isUpgrade,
        const std::string &exitMsg);

    int32_t GetMissionIdByAbilityTokenInner(const sptr<IRemoteObject> &token);
    bool CheckCallerIsDmsProcess();

    void WaitBootAnimationStart();

    struct SignRestartAppFlagParam {
        int32_t userId;
        int32_t uid;
        std::string instanceKey;
        AppExecFwk::MultiAppModeType type;
        bool isAppRecovery = false;
        bool isAtomicService = false;
    };
    int32_t SignRestartAppFlag(const SignRestartAppFlagParam &param);

    int32_t CheckRestartAppWant(const AAFwk::Want &want, int32_t appIndex, int32_t userId);

    int32_t CheckDebugAssertPermission();
    bool VerifySameAppOrAppIdentifierAllowListPermission(const AbilityRequest &abilityRequest);
    std::shared_ptr<AbilityDebugDeal> ConnectInitAbilityDebugDeal();

    int32_t SetBackgroundCall(const AppExecFwk::RunningProcessInfo &processInfo,
        const AbilityRequest &abilityRequest, bool &isBackgroundCall) const;

    int CheckUIExtensionUsage(AppExecFwk::UIExtensionUsage uiExtensionUsage,
        AppExecFwk::ExtensionAbilityType extensionType);

    bool CheckUIExtensionCallerIsForeground(const AbilityRequest &abilityRequest);
    bool CheckStartCallHasFloatingWindowForUIExtension(const sptr<IRemoteObject> &callerToken);
    bool CheckUIExtensionCallerIsUIAbility(const AbilityRequest &abilityRequest);
    std::shared_ptr<AbilityRecord> GetUIExtensionRootCaller(const sptr<IRemoteObject> token, int32_t userId);

    bool CheckUIExtensionCallerPidByHostWindowId(const AbilityRequest &abilityRequest, int32_t validUserId);

    int CheckExtensionCallPermission(const Want& want, const AbilityRequest& abilityRequest, uint32_t specifyTokenId);

    int CheckServiceCallPermission(const AbilityRequest& abilityRequest,
        const AppExecFwk::AbilityInfo& abilityInfo);

    int CheckBrokerCallPermission(const AbilityRequest& abilityRequest,
        const AppExecFwk::AbilityInfo& abilityInfo);

    int CheckAbilityCallPermission(const AbilityRequest& abilityRequest, const AppExecFwk::AbilityInfo& abilityInfo,
        uint32_t specifyTokenId, bool isSelector, bool isFreeInstallFromService = false);

    int CheckCallPermission(const Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
        const AbilityRequest& abilityRequest, bool isForegroundToRestartApp,
        bool isSendDialogResult, uint32_t specifyTokenId,
        const std::string& callerBundleName, bool isSelector, bool isFreeInstallFromService = false);

    void CheckExtensionRateLimit(const Want &want);

    int32_t CheckStartPlugin(const Want& want, sptr<IRemoteObject> callerToken, bool &isTargetPlugin);

    int StartAbilityByConnectManager(const Want& want, const AbilityRequest& abilityRequest,
        const AppExecFwk::AbilityInfo& abilityInfo, int validUserId, sptr<IRemoteObject> callerToken);

    int PreStartFreeInstall(const Want &want, sptr<IRemoteObject> callerToken,
        uint32_t specifyTokenId, bool isStartAsCaller, Want &localWant);

    void ReportCleanSession(const sptr<SessionInfo> &sessionInfo,
        const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t errCode);

    void SendStartAbilityOtherExtensionEvent(const AppExecFwk::AbilityInfo& abilityInfo,
        const Want& want, uint32_t specifyTokenId);

    void SetMinimizedDuringFreeInstall(const sptr<SessionInfo>& sessionInfo);

    bool CheckWorkSchedulerPermission(const sptr<IRemoteObject> &callerToken, const uint32_t uid);

    /**
     * @brief Check debug app in developer mode.
     * @param applicationInfo. The application info.
     * @return Returns true or false.
     */
    bool CheckDebugAppNotInDeveloperMode(const AppExecFwk::ApplicationInfo &applicationInfo);

    /**
     * @brief Prompt user that developer mode has not been turned on.
     * @param bundleName. The bundleName of the blocked hap.
     * @param abilityName. The abilityName of the blocked hap.
     */
    void ShowDeveloperModeDialog(const std::string &bundleName, const std::string &abilityName);

    void StartKeepAliveAppsInner(int32_t userId);

    bool ProcessLowMemoryKill(int32_t pid, const ExitReason &reason, bool isKillPrecedeStart);

    void TimeSequenceKeepAliveRestart(int32_t userId, pid_t pid,
        std::map<int32_t, std::vector<AppExecFwk::BundleInfo>> &bundleInfosMap,
        std::vector<AppExecFwk::BundleInfo> &bundleInfosForU1);

    struct StartSelfUIAbilityParam {
        Want want;
        StartOptions options;
        bool hasStartOptions = false;
    };
    int StartSelfUIAbilityInner(StartSelfUIAbilityParam param);

    bool HandleExecuteSAInterceptor(const Want &want, sptr<IRemoteObject> callerToken,
        AbilityRequest &abilityRequest, int32_t &result);

    bool controllerIsAStabilityTest_ = false;
    bool isParamStartAbilityEnable_ = false;
    // Component StartUp rule switch
    bool startUpNewRule_ = true;
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
    bool shouldBlockAllAppStart_ = false;

    int32_t uniqueId_ = 0;
    std::unordered_set<int32_t> windowVisibleList_;

    sptr<WindowVisibilityChangedListener> windowVisibilityChangedListener_;
    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AbilityEventHandler> eventHandler_;
    ServiceRunningState state_;

    std::shared_ptr<FreeInstallManager> freeInstallManager_;
    std::shared_ptr<SubManagersHelper> subManagersHelper_;
    sptr<AppExecFwk::IAbilityController> abilityController_ = nullptr;

    std::multimap<std::string, std::string> timeoutMap_;
    std::map<std::string, sptr<SessionInfo>> preStartSessionMap_;

    static sptr<AbilityManagerService> instance_;
    std::map<int32_t, std::pair<int64_t, const sptr<IAcquireShareDataCallback>>> iAcquireShareDataMap_;
    // first is callstub, second is ability token
    std::map<sptr<IRemoteObject>, sptr<IRemoteObject>> callStubTokenMap_;
#ifdef SUPPORT_GRAPHICS
    sptr<WindowFocusChangedListener> focusListener_;
#endif // SUPPORT_GRAPHICS

    std::shared_ptr<AbilityRuntime::AbilityUserUnlockEventSubscriber> userUnlockSubscriber_;
    std::shared_ptr<AbilityRuntime::AbilityScreenUnlockEventSubscriber> screenSubscriber_;
    bool isSubscribed_ = false;
    std::mutex subscribedMutex_;

    std::shared_ptr<AbilityAutoStartupService> abilityAutoStartupService_;

    std::map<std::string, std::list<std::string>> whiteListMap_;

    std::list<std::string> exportWhiteList_;

    std::shared_ptr<AbilityRuntime::InsightIntentEventMgr> insightIntentEventMgr_;

    bool ShouldPreventStartAbility(const AbilityRequest &abilityRequest);

    void PrintStartAbilityInfo(AppExecFwk::AbilityInfo callerInfo, AppExecFwk::AbilityInfo calledInfo);

    bool IsInWhiteList(const std::string &callerBundleName, const std::string &calleeBundleName,
        const std::string &calleeAbilityName);

    std::string GetConfigFileAbsolutePath(const std::string &relativePath);

    int32_t ParseJsonValueFromFile(nlohmann::json &value, const std::string& fullPath);

    bool ConvertFullPath(const std::string& partialPath, std::string& fullPath);

    bool ParseJsonFromBoot(const std::string &relativePath);

    void SetReserveInfo(const std::string &linkString, AbilityRequest& abilityRequest);
    void CloseAssertDialog(const std::string &assertSessionId, int32_t userId);

    int32_t OpenLinkFreeInstallAtomicService(StartAbilityWrapParam &param, const Want &originalWant);

    void ReportPreventStartAbilityResult(const AppExecFwk::AbilityInfo &callerAbilityInfo,
        const AppExecFwk::AbilityInfo &abilityInfo);

    void SetAbilityRequestSessionInfo(AbilityRequest &abilityRequest, AppExecFwk::ExtensionAbilityType extensionType);

    bool ShouldBlockAllAppStart();

    std::string InsightIntentGetcallerBundleName();

    ErrCode IntentOpenLinkInner(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        AbilityRuntime::ExtractInsightIntentGenericInfo &linkInfo, const int32_t userId);
 
    AbilityRuntime::ExtractInsightIntentGenericInfo GetInsightIntentGenericInfo(const InsightIntentExecuteParam &param);
 
    void CombinLinkInfo(
        const std::vector<AbilityRuntime::LinkIntentParamMapping> &paramMappings, std::string &uri, AAFwk::Want &want);

    int32_t UpdateApplicationKeepAlive(int32_t userId) const;

    int32_t OpenLinkInner(const Want &want, sptr<IRemoteObject> callerToken, int32_t userId, int requestCode,
        bool removeInsightIntentFlag, bool hideFailureTipDialog = false);
    int32_t KillProcessWithReasonInner(int32_t pid, const ExitReason &reason, bool isKillPrecedeStart);

    int32_t ProcessUdmfKey(
        const Want &want, uint32_t targetTokenId, AppExecFwk::ExtensionAbilityType extensionType);

    bool IsAllowAttachOrDetachAppDebug(AppExecFwk::ApplicationInfo &appInfo);
    bool IsExitReasonValid(const ExitReasonCompability &reason);
    void RecordRecoveryExitReason(bool isAppRecovery, int32_t callerPid, int32_t callerUid);
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::shared_ptr<BackgroundTaskObserver> bgtaskObserver_;
#endif

    sptr<AbilityBundleEventCallback> abilityBundleEventCallback_;

#ifdef SUPPORT_SCREEN
    bool isAnimationEnabled_ = true; //only use on mission list
    bool CheckWindowMode(int32_t windowMode, const std::vector<AppExecFwk::SupportWindowMode>& windowModes) const;
    int32_t ShowPickerDialog(const Want& want, int32_t userId, const sptr<IRemoteObject> &token);
    void InitFocusListener();
    void RegisterFocusListener();
    std::shared_ptr<ImplicitStartProcessor> implicitStartProcessor_;
    ffrt::mutex wmsHandlerLock_;
    sptr<IWindowManagerServiceHandler> wmsHandler_;
#endif
    std::shared_ptr<AbilityInterceptorExecuter> interceptorExecuter_;
    std::shared_ptr<AbilityInterceptorExecuter> afterCheckExecuter_;

    std::unordered_map<int32_t, int64_t> appRecoveryHistory_; // uid:time
    std::unordered_map<int32_t, sptr<IAbilityManagerCollaborator>> collaboratorMap_;

    std::shared_ptr<AbilityDebugDeal> abilityDebugDeal_;
    std::shared_ptr<AppExitReasonHelper> appExitReasonHelper_;

    int32_t rFd_ = -1;
    int32_t wFd_ = -1;
    std::shared_ptr<int32_t> ptrRFd_ = std::make_shared<int32_t>(-1);
    ffrt::mutex globalLock_;
    ffrt::mutex bgtaskObserverMutex_;
    ffrt::mutex abilityTokenLock_;
    ffrt::mutex preStartSessionMapLock_;
    ffrt::mutex windowVisibleListLock_;
    ffrt::mutex iAcquireShareDataMapLock_;
    ffrt::mutex collaboratorMapLock_;
    ffrt::mutex abilityDebugDealLock_;
    ffrt::mutex shouldBlockAllAppStartMutex_;
    mutable ffrt::mutex timeoutMapLock_;
    std::mutex whiteListMutex_;

    std::mutex prepareTermiationCallbackMutex_;
    std::map<std::string, sptr<IPrepareTerminateCallback>> prepareTermiationCallbacks_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
