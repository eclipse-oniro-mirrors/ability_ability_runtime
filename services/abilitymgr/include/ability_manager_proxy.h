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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_PROXY_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_PROXY_H

#include "ability_manager_interface.h"
#include "auto_startup_info.h"
#include "iremote_proxy.h"
#include "mission_info.h"
#include "intent_exemption_info.h"

namespace OHOS {
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
/**
 * @class AbilityManagerProxy
 * AbilityManagerProxy.
 */
class AbilityManagerProxy : public IRemoteProxy<IAbilityManager> {
public:
    explicit AbilityManagerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAbilityManager>(impl)
    {}

    virtual ~AbilityManagerProxy()
    {}

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
        const Want &want,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        uint64_t specifiedFullTokenId = 0) override;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @param userId, Designation User ID.
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
     * @param callerToken caller ability token.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @param userId, Designation User ID.
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
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param callerToken current caller ability token.
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
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken current caller ability token.
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
    ErrCode StartUIAbilitiesInSplitWindowMode(int32_t primaryWindowId, const AAFwk::Want &secondaryWant,
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
        int requestCode = DEFAULT_INVAL_VALUE) override;

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
        int requestCode = DEFAULT_INVAL_VALUE) override;

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
     * Create UIExtension with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RequestModalUIExtension(const Want &want) override;

    /**
     * Preload UIExtension with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param hostBundleName, the caller application bundle name.
     * @param userId, the extension runs in.
     * @param requestCode the resultCode of the preload ui extension ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PreloadUIExtensionAbility(const Want &want, std::string &hostBundleName,
        int32_t userId = DEFAULT_INVAL_VALUE, int32_t hostPid = DEFAULT_INVAL_VALUE,
        int32_t requestCode = DEFAULT_INVAL_VALUE) override;

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
    virtual int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) override;

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
     * TerminateUIServiceExtensionAbility, terminate UIServiceExtensionAbility.
     *
     * @param token, the token of the UIServiceExtensionAbility to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t TerminateUIServiceExtensionAbility(const sptr<IRemoteObject> &token) override;

    /**
     * TerminateUIExtensionAbility, terminate the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to terminate.
     * @param resultCode resultCode.
     * @param Want Ability want returned.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int resultCode,
        const Want *resultWant) override;

    /**
     * CloseUIExtensionAbilityBySCB, terminate the specified ui extension ability by SCB.
     *
     * @param token the ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CloseUIExtensionAbilityBySCB(const sptr<IRemoteObject> token) override;

    /**
     * CloseUIAbilityBySCB, close the special ability by scb.
     *
     * @param sessionInfo the session info of the ability to terminate.
     * @param sceneFlag the reason info of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool isUserRequestedExit,
        uint32_t sceneFlag = 0) override;

    /**
     * SendResultToAbility with want, return want from ability manager service.(Only used for dms)
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
     * @param token, the token of the ability to move.
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
    virtual int CloseAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) override;

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
     * @param sessionInfo the session info of the ability to minimize.
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
     * DisconnectAbility, connect session with service ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbility(sptr<IAbilityConnection> connect) override;

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param isKill, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool isKill, const sptr<IRemoteObject> &callerToken) override;

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityScheduler, specifies the data ability that will be released.
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
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual void DumpState(const std::string &args, std::vector<std::string> &state) override;
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID) override;

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
     * Get top ability.
     *
     * @param isNeedLocalDeviceId is need local device id.
     * @return Returns front desk focus ability elementName.
     */
    virtual AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId = true) override;

    /**
     * Get element name by token.
     *
     * @param token ability's token.
     * @param isNeedLocalDeviceId is need local device id.
     * @return Returns front desk focus ability elementName by token.
     */
    virtual AppExecFwk::ElementName GetElementNameByToken(sptr<IRemoteObject> token,
        bool isNeedLocalDeviceId = true) override;

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillProcess(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0,
        const std::string& reason = "Abilityms::KillProcess") override;

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

    virtual int SendWantSender(sptr<IWantSender> target, SenderInfo &senderInfo) override;

    virtual int SendLocalWantSender(const SenderInfo &senderInfo) override;

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

    virtual int GetAppMemorySize() override;

    virtual bool IsRamConstrainedDevice() override;
    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override;

    virtual int ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo,
        const sptr<IRemoteObject> &callback) override;

    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) override;

    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override;

    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override;

    virtual int NotifyContinuationResult(int32_t missionId, int32_t result) override;

    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;

    virtual int StopSyncRemoteMissions(const std::string& devId) override;

    virtual int LockMissionForCleanup(int32_t missionId) override;

    virtual int UnlockMissionForCleanup(int32_t missionId) override;

    virtual void SetLockedState(int32_t sessionId, bool lockedState) override;

    /**
     * @brief Register mission listener to ability mgr.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    /**
     * @brief UnRegister mission listener from ability mgr.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    /**
     * @brief Get mission infos from ability mgr.
     * @param deviceId local or remote deviceId.
     * @param numMax max number of missions.
     * @param missionInfos mission info result.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override;

    /**
     * @brief Get mission info by id.
     * @param deviceId local or remote deviceId.
     * @param missionId Id of target mission.
     * @param missionInfo mission info of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfos) override;

    /**
     * @brief Clean mission by id.
     * @param missionId Id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CleanMission(int32_t missionId) override;

    /**
     * @brief Clean all missions in system.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CleanAllMissions() override;

    virtual int MoveMissionToFront(int32_t missionId) override;

    /**
     * @brief Move a mission to front.
     * @param missionId Id of target mission.
     * @param startOptions Special startOptions for target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) override;

    /**
     * Move missions to front
     * @param missionIds Ids of target missions
     * @param topMissionId Indicate which mission will be moved to top, if set to -1, missions' order won't change
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override;

    /**
     * Move missions to background
     * @param missionIds Ids of target missions
     * @param result The result of move missions to background, and the array is sorted by zOrder
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
        std::vector<int32_t>& result) override;

    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken Indicates the caller's identity
     * @param accountId Indicates the account to start.
     * @param isSilent, whether show window when start fail.
     * @param promotePriority, whether to promote priority for sa.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId = DEFAULT_INVAL_VALUE,
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
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken Indicates the caller's identity
     * @param accountId Indicates the account to start.
     * @param errMsg Out parameter, indicates the failed reason.
     * @param isSilent, whether show window when start fail.
     * @param promotePriority, whether to promote priority for sa.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCallWithErrMsg(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId, std::string &errMsg,
        bool isSilent = false, bool promotePriority = false, bool isVisible = false) override;

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

    /**
     * @brief start user.
     * @param accountId accountId.
     * @param displayId logical screen id.
     * @param isAppRecovery is appRecovery or not.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback,
        bool isAppRecovery = false) override;

    /**
     * @brief stop user.
     * @param accountId accountId.
     * @param callback callback.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopUser(int userId, const sptr<IUserCallback> &callback) override;

    /**
     * @brief logout user.
     * @param accountId accountId.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int LogoutUser(int32_t userId, sptr<IUserCallback> callback) override;

    virtual int SetMissionContinueState(const sptr<IRemoteObject> &token, const AAFwk::ContinueState &state) override;

#ifdef SUPPORT_SCREEN
    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) override;

    virtual int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;

    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
        bool animationEnabled) override;

    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override;

    virtual void CompleteFirstFrameDrawing(int32_t sessionId) override;

    virtual int PrepareTerminateAbility(
        const sptr<IRemoteObject> &token, sptr<IPrepareTerminateCallback> &callback) override;

    virtual int GetDialogSessionInfo(const std::string &dialogSessionId, sptr<DialogSessionInfo> &info) override;

    virtual int SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllow) override;

    virtual int RegisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer,
        const std::string &targetBundleName) override;

    virtual int UnregisterAbilityFirstFrameStateObserver(
        const sptr<IAbilityFirstFrameStateObserver> &observer) override;
#endif
    /**
     * @brief Get the ability running information.
     *
     * @param info Ability running information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override;

    /**
     * @brief Get the extension running information.
     *
     * @param upperLimit The maximum limit of information wish to get.
     * @param info Extension running information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override;

    /**
     * @brief Get running process information.
     *
     * @param info Running process information.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override;

    /**
     * @brief Register mission listener to ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;

    virtual int RegisterOnListener(const std::string &type,
        const sptr<IRemoteOnListener> &listener) override;

    virtual int RegisterOffListener(const std::string &deviceId,
        const sptr<IRemoteOnListener> &listener) override;

    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;

    /**
     * Set ability controller.
     *
     * @param abilityController, The ability controller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override;

    /**
     * Is user a stability test.
     *
     * @return Returns true if user is a stability test.
     */
    virtual bool IsRunningInStabilityTest() override;

    /**
     * @brief Register the snapshot handler
     * @param handler snapshot handler
     * @return ErrCode Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;

    /**
     * @brief Get the Mission Snapshot Info object
     * @param deviceId local or remote deviceId.
     * @param missionId Id of target mission.
     * @param snapshot snapshot of target mission.
     * @param isLowResolution get low resolution snapshot.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;

    /**
     * @brief start user test.
     * @param want the want of the ability user test to start.
     * @param observer test observer callback.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) override;

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
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

    /**
     * Get mission id by ability token.
     *
     * @param token The token of ability.
     * @return Returns -1 if do not find mission, otherwise return mission id.
     */
    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) override;

    /**
     * Get ability token by connect.
     *
     * @param token The token of ability.
     * @param callStub The callee object.
     */
    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) override;

    /**
     * Call free install from remote.
     *
     * @param want, the want of the ability to start.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @param callback, Callback from remote.
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
     * Called when client complete dump.
     *
     * @param infos The dump info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken) override;

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

    /**
     * @brief Schedule clear recovery page stack.
     *
     * @param bundleName application bundleName.
     */
    virtual void ScheduleClearRecoveryPageStack() override;

    /**
     * Called to verify that the MissionId is valid.
     * @param missionIds Query mission list.
     * @param results Output parameters, return results up to 20 query results.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t IsValidMissionIds(
        const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results) override;

    /**
     * Query whether the application of the specified PID and UID has been granted a certain permission
     * @param permission
     * @param pid Process id
     * @param uid
     * @return Returns ERR_OK if the current process has the permission, others on failure.
     */
    virtual int VerifyPermission(const std::string &permission, int pid, int uid) override;

    /**
     * Request dialog service with want, send want to ability manager service.
     *
     * @param want, the want of the dialog service to start.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken) override;

    int32_t ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken) override;

    virtual int32_t AcquireShareData(
        const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData) override;
    virtual int32_t ShareDataDone(const sptr<IRemoteObject> &token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam) override;

    /**
     * Force app exit and record exit reason.
     * @param pid Process id .
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ForceExitApp(const int32_t pid, const ExitReason &exitReason) override;

    /**
     * Record app exit reason.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RecordAppExitReason(const ExitReason &exitReason) override;

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
    virtual int32_t KillAppWithReason(const int32_t pid, const ExitReasonCompability &exitReason) override;

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
    virtual void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, AbilityRuntime::StartParamsBySCB &params,
        bool &isColdStart) override;

    /**
     * Start specified ability by SCB.
     *
     * @param want Want information.
     * @param params The parameters to start specified ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartSpecifiedAbilityBySCB(const Want &want, const StartSpecifiedAbilityParams &params) override;

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

    virtual int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate) override;

    virtual int32_t KillProcessWithPrepareTerminate(const std::vector<int32_t> &pids, bool clear) override;

    /**
     * @brief  the process with reason
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
     * @param isPrepareTerminate the result of ability onPrepareToTerminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PrepareTerminateAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isPrepareTerminate) override;

    /**
     * @brief Register session handler.
     * @param object The handler.
     *
     * @return Returns ERR_OK on success, others on failure.
    */
    virtual int RegisterSessionHandler(const sptr<IRemoteObject> &object) override;

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
     * @brief Open file by uri.
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @return int The file descriptor.
     */
    virtual int32_t OpenFile(const Uri& uri, uint32_t flag) override;

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
     * @return int32_t Returns 0 on success, others on failure.
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
     * @brief Pop-up launch of full-screen atomic service.
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
     * @param callerToken caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @param want Indicates the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t TransferAbilityResultForExtension(const sptr<IRemoteObject> &callerToken, int32_t resultCode,
        const Want &want) override;

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

    /**
     *  Request to clean UIAbility from user.
     *
     * @param sessionInfo the session info of the ability to clean.
     * @param sceneFlag the reason info of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CleanUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool isUserRequestedExit,
        uint32_t sceneFlag = 0) override;

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
        int requestCode = DEFAULT_INVAL_VALUE, bool hideFailureTipDialog = false) override;

    /**
     * Terminate the mission.
     *
     * @param missionId, The mission id of the UIAbility need to be terminated.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t TerminateMission(int32_t missionId) override;

    /**
     * Notify ability manager to set the flag to block all apps from starting.
     * Needs to apply for ohos.permission.BLOCK_ALL_APP_START.
     * @param flag, The flag to block all apps from starting
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t BlockAllAppStart(bool flag) override;

    /**
     * update associate config list by rss.
     *
     * @param configs The rss config info.
     * @param exportConfigs The rss export config info.
     * @param flag UPDATE_CONFIG_FLAG_COVER is cover config, UPDATE_CONFIG_FLAG_APPEND is append config.
     */
    virtual int32_t UpdateAssociateConfigList(const std::map<std::string, std::list<std::string>>& configs,
        const std::list<std::string>& exportConfigs, int32_t flag) override;

    /**
     * Set keep-alive flag for application under a specific user.
     * @param bundleName Bundle name.
     * @param userId User Id.
     * @param flag Keep-alive flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetApplicationKeepAlive(const std::string &bundleName, int32_t userId, bool flag) override;

    /**
     * Get keep-alive applications.
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
     * Get intent exemption info.
     * @param list List of intent exemption info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAllIntentExemptionInfo(std::vector<AppExecFwk::IntentExemptionInfo> &info) override;

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

    /**
     * KillProcessForPermissionUpdate
     * force kill the application by accessTokenId, notify exception to SCB.
     *
     * @param  accessTokenId, accessTokenId.
     * @return ERR_OK, return back success, others fail.
     */
    virtual ErrCode KillProcessForPermissionUpdate(uint32_t accessTokenId) override;

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
    int32_t StartAbilityWithWait(Want &want, sptr<IAbilityStartWithWaitObserver> &observer) override;

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

    int32_t UpdateKioskApplicationList(const std::vector<std::string> &appList) override;

    int32_t EnterKioskMode(sptr<IRemoteObject> callerToken) override;

    int32_t ExitKioskMode(sptr<IRemoteObject> callerToken) override;

    int32_t GetKioskStatus(AAFwk::KioskStatus &kioskStatus) override;

    /**
     * Register sa interceptor.
     * @param interceptor, The sa interceptor.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode RegisterSAInterceptor(sptr<AbilityRuntime::ISAInterceptor> interceptor) override;

    /**
     * SuspendExtensionAbility, suspend session with service ability.
     *
     * @param connect, Callback used to notify caller the result of suspend or resume.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SuspendExtensionAbility(sptr<IAbilityConnection> connect) override;

    /**
     * ResumeExtensionAbility, resume session with service ability.
     *
     * @param connect, Callback used to notify caller the result of suspend or resume.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ResumeExtensionAbility(sptr<IAbilityConnection> connect) override;

    virtual int32_t SetOnNewWantSkipScenarios(sptr<IRemoteObject> callerToken, int32_t scenarios) override;

    virtual int32_t NotifyStartupExceptionBySCB(int32_t requestId) override;

    /**
     * Preload application.
     * @param bundleName Name of the application.
     * @param userId user id.
     * @param appIndex app clone index. Reserved field, only appIndex=0 is supported.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t PreloadApplication(const std::string &bundleName, int32_t userId, int32_t appIndex) override;

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
     * Check if the app is restart-limited.
     * @return Returns true on being limited.
     */
    virtual bool IsRestartAppLimit() override;

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
     * @param hostBundleName, the caller application bundle name.
     * @param hostPid, the caller hostPid.
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
    template <typename T>
    int GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos);
    bool WriteInterfaceToken(MessageParcel &data);
    // flag = true : terminate; flag = false : close
    int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant, bool flag);
    ErrCode SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data, MessageParcel &reply,
        MessageOption& option);
    int CheckUISessionParams(MessageParcel &data, const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo, int32_t userId, int requestCode);
    bool UpdateAssociateConfigInner(const std::map<std::string, std::list<std::string>>& configs,
        MessageParcel& data);
    bool ExtendMaxIpcCapacityForWant(const Want &want, MessageParcel &data);

private:
    static inline BrokerDelegator<AbilityManagerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
