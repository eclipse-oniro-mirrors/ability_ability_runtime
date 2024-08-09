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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H

#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include "cpp/mutex.h"

#include "ability_cache_manager.h"
#include "ability_connect_callback_interface.h"
#include "task_handler_wrap.h"
#include "event_handler_wrap.h"
#include "ability_record.h"
#include "ability_running_info.h"
#include "event_report.h"
#include "extension_config.h"
#include "extension_running_info.h"
#include "connection_record.h"
#include "element_name.h"
#include "ui_extension_ability_connect_info.h"
#include "extension_record_manager.h"
#include "want.h"
#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::AbilityType;
using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;
using UIExtensionAbilityConnectManager = AbilityRuntime::ExtensionRecordManager;
using UIExtensionSessionInfo = AbilityRuntime::UIExtensionSessionInfo;
/**
 * @class AbilityConnectManager
 * AbilityConnectManager provides a facility for managing service ability connection.
 */
class AbilityConnectManager : public std::enable_shared_from_this<AbilityConnectManager> {
public:
    using ConnectMapType = std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<ConnectionRecord>>>;
    using ServiceMapType = std::map<std::string, std::shared_ptr<AbilityRecord>>;
    using ConnectListType = std::list<std::shared_ptr<ConnectionRecord>>;
    using RecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;
    using UIExtWindowMapValType = std::pair<std::weak_ptr<AbilityRecord>, sptr<SessionInfo>>;
    using UIExtensionMapType = std::map<sptr<IRemoteObject>, UIExtWindowMapValType>;
    using WindowExtMapValType = std::pair<uint32_t, sptr<SessionInfo>>;
    using WindowExtensionMapType = std::map<sptr<IRemoteObject>, WindowExtMapValType>;

    explicit AbilityConnectManager(int userId);
    virtual ~AbilityConnectManager();

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbility(const AbilityRequest &abilityRequest);

    /**
     * TerminateAbility with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility(const sptr<IRemoteObject> &token);

    /**
     * StopServiceAbility with request.
     *
     * @param abilityRequest, request.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StopServiceAbility(const AbilityRequest &abilityRequest);

    /**
     * ConnectAbilityLocked, connect session with service ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param sessionInfo the extension session info of the ability to connect.
     * @param connectInfo the connect info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr);

    /**
     * PreloadUIExtensionAbilityInner, preload uiextension ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param hostBundleName, the caller application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest, std::string &hostBundleName);

    /**
     * PreloadUIExtensionAbilityLocked, preload uiextension ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param hostBundleName, the caller application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest, std::string &hostBundleName);

    /**
     * UnloadUIExtensionAbility, unload uiextension ability.
     *
     * @param abilityRecord, uiextension ability record.
     * @param hostBundleName, the caller application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, std::string &bundleName);

    /**
     * ClearPreloadUIExtensionRecord, clear preload uiextension record.
     *
     * @param abilityRecord, uiextension ability record.
     */
    void ClearPreloadUIExtensionRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * AttachAbilityThreadLocked, ability call this interface after loaded.
     *
     * @param scheduler, the interface handler of kit ability.
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state);

    void OnAppStateChanged(const AppInfo &info);

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token, ability's token.
     * @param state, the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state);

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param windowConfig the windowconfig
     * @return execute error code
     */
    int AbilityWindowConfigTransactionDone(
        const sptr<IRemoteObject> &token, const AppExecFwk::WindowConfig &windowConfig);

    /**
     * ScheduleConnectAbilityDoneLocked, service ability call this interface while session was connected.
     *
     * @param token, service ability's token.
     * @param remoteObject, the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject);

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token);

    /**
     * ScheduleCommandAbilityDoneLocked, service ability call this interface while session was onCommanded.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token);

    int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd);

    /**
     * GetUIExtensioBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetUIExtensioBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    std::shared_ptr<AbilityRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token);
    std::shared_ptr<AbilityRecord> GetExtensionByTokenFromAbilityCache(const sptr<IRemoteObject> &token);
    std::shared_ptr<AbilityRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token);
    ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    void GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
        const int32_t userId, bool isPerm);

    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm);

    void GetExtensionRunningInfo(std::shared_ptr<AbilityRecord> &abilityRecord, const int32_t userId,
        std::vector<ExtensionRunningInfo> &info);

    /**
     * set from ability manager service for sequenced task
     */
    inline void SetTaskHandler(const std::shared_ptr<TaskHandlerWrap> &taskHandler)
    {
        taskHandler_ = taskHandler;
    }
    /**
     * SetEventHandler.
     *
     * @param handler,EventHandler
     */
    inline void SetEventHandler(const std::shared_ptr<EventHandlerWrap> &handler)
    {
        eventHandler_ = handler;
    }

    uint32_t GetSceneBoardTokenId() const
    {
        return sceneBoardTokenId_;
    }

    /**
     * @brief Get extensionList by pid.
     * @param pid Process id.
     * @param extensionList UIExtensionAbility name list.
     */
    int32_t GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList);

    /**
     * @brief Get extensionList by bundleName.
     * @param bundleName The application bundle name.
     * @param extensionList UIExtensionAbility name list.
     */
    int32_t GetActiveUIExtensionList(const std::string &bundleName, std::vector<std::string> &extensionList);

    /**
     * OnAbilityDied.
     *
     * @param abilityRecord, service ability record.
     */
    void OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);

    void DumpState(std::vector<std::string> &info, bool isClient, const std::string &args = "");

    void DumpStateByUri(std::vector<std::string> &info, bool isClient, const std::string &args,
        std::vector<std::string> &params);

    void PauseExtensions();

    void StartRootLauncher(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId);

    void MoveToForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    void MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void CommandAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);

    bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken);

    sptr<IRemoteObject> GetUIExtensionSourceToken(const sptr<IRemoteObject> &token);

    bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken);

    void HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid);

    void ForegroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void BackgroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void TerminateAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void RemoveLauncherDeathRecipient();

    /**
     * @brief Get ui extension session info
     *
     * @param token The ability token.
     * @param uiExtensionSessionInfo The ui extension session info.
     * @param userId The user id.
     * @return int32_t Returns ERR_OK on success, others on failure.
     */
    int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo);

    void CloseAssertDialog(const std::string &assertSessionId);

    void SignRestartAppFlag(const std::string &bundleName);

    std::shared_ptr<AAFwk::AbilityRecord> GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token);

    void DeleteInvalidServiceRecord(const std::string &bundleName);

    // MSG 0 - 20 represents timeout message
    static constexpr uint32_t LOAD_TIMEOUT_MSG = 0;
    static constexpr uint32_t CONNECT_TIMEOUT_MSG = 1;

private:
    /**
     * StartAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbilityLocked(const AbilityRequest &abilityRequest);

    /**
     * TerminateAbilityLocked with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @param resultCode, the result code of service type's ability to terminate.
     * @param resultWant, the result want for service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbilityLocked(const sptr<IRemoteObject> &token);

    /**
     * StopAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StopServiceAbilityLocked(const AbilityRequest &abilityRequest);

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerDied, bool Indicates if it is caused by the caller's death.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool callerDied);

    /**
     * LoadAbility.
     *
     * @param abilityRecord, the ptr of the ability to load.
     */
    void LoadAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectUIServiceExtAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int connectRecordId, const Want &want);

    /**
     * ConnectAbility.Schedule Resume Connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ResumeConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CommandAbility. Schedule command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CommandAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CompleteCommandAbility. complete command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CompleteCommandAbility(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * TerminateDone.
     *
     * @param abilityRecord, the ptr of the ability to terminate.
     */
    void TerminateDone(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * GetServiceRecordByElementName.
     *
     * @param element, service ability's element.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetServiceRecordByElementName(const std::string &element);

    /**
     * dispatch service ability life cycle .
     *
     * @param abilityRecord.
     * @param state.
     */
    int DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void HandleStartTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleStopTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleTerminateDisconnectTask(const ConnectListType& connectlist);
    void HandleCommandTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleCommandWindowTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);
    void HandleForegroundTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleConnectTimeoutTask(std::shared_ptr<AbilityRecord> abilityRecord);
    void HandleRestartResidentTask(const AbilityRequest &abilityRequest);
    void HandleActiveAbility(std::shared_ptr<AbilityRecord> &targetService,
        std::shared_ptr<ConnectionRecord> &connectRecord);
    void HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo);
    void TerminateOrCacheAbility(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * IsAbilityConnected.
     *
     * @param abilityRecord, the ptr of the connected ability.
     * @param connectRecordList, connect record list.
     * @return true: ability is connected, false: ability is not connected
     */
    bool IsAbilityConnected(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);

    /**
     * RemoveConnectionRecordFromMap.
     *
     * @param connect, the ptr of the connect record.
     */
    void RemoveConnectionRecordFromMap(std::shared_ptr<ConnectionRecord> connect);

    /**
     * RemoveServiceAbility.
     *
     * @param service, the ptr of the ability record.
     */
    void RemoveServiceAbility(const std::shared_ptr<AbilityRecord> &service);

    /**
     * GetOrCreateServiceRecord.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param targetAbilityRecord, the target service ability record.
     * @param isLoadedAbility, whether the target ability has been loaded.
     */
    void GetOrCreateServiceRecord(const AbilityRequest &abilityRequest, const bool isCreatedByConnect,
        std::shared_ptr<AbilityRecord> &targetAbilityRecord, bool &isLoadedAbility);

    /**
     * GetConnectRecordListFromMap.
     *
     * @param connect, callback object.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param connectRecordList, the target connectRecordList.
     * @param isCallbackConnected, whether the callback has been connected.
     */
    void GetConnectRecordListFromMap(
        const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);

    /**
     * AddConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    void AddConnectDeathRecipient(sptr<IRemoteObject> connectObject);

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    void RemoveConnectDeathRecipient(sptr<IRemoteObject> connectObject);

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param remote, callback object.
     */
    void OnCallBackDied(const wptr<IRemoteObject> &remote);

    /**
     * HandleOnCallBackDied.
     *
     * @param connect, callback object.
     */
    void HandleCallBackDiedTask(const sptr<IRemoteObject> &connect);

    /**
     * HandleOnCallBackDied.
     *
     * @param abilityRecord, died ability.
     */
    void HandleAbilityDiedTask(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);
    void HandleUIExtensionDied(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void RestartAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);

    /**
     * PostTimeOutTask.
     *
     * @param abilityRecord, ability.
     * @param messageId, message id.
     */
    void PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t messageId);
    void PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, int connectRecordId, uint32_t messageId);

    void CompleteForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId);

    void PostRestartResidentTask(const AbilityRequest &abilityRequest);

    bool IsAbilityNeedKeepAlive(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const;

    std::shared_ptr<AbilityRecord> GetAbilityRecordById(int64_t abilityRecordId);
    void HandleInactiveTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void MoveToTerminatingMap(const std::shared_ptr<AbilityRecord>& abilityRecord);

    void DoForegroundUIExtension(std::shared_ptr<AbilityRecord> abilityRecord, const AbilityRequest &abilityRequest);
    void DoBackgroundAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    /**
     * When a service is under starting, enque the request and handle it after the service starting completes
     */
    void EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri = "");
    /**
     * After the service starting completes, complete the request list
     */
    void CompleteStartServiceReq(const std::string &serviceUri);

    void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);
    void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);
    void OnUIExtWindowDied(const wptr<IRemoteObject> &remote);
    void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote);

    /**
     * Post an extension's disconnect task, auto disconnect when extension conected timeout.
     */
    void PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Remove the extension's disconnect task.
     */
    void RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Handle extension disconnect task.
     */
    void HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

private:
    void TerminateRecord(std::shared_ptr<AbilityRecord> abilityRecord);
    int DisconnectRecordNormal(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord,
        bool callerDied) const;
    void DisconnectRecordForce(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord);
    std::shared_ptr<AbilityRecord> GetExtensionByIdFromServiceMap(int32_t abilityRecordId);
    int TerminateAbilityInner(const sptr<IRemoteObject> &token);
    bool IsLauncher(std::shared_ptr<AbilityRecord> serviceExtension) const;
    void KillProcessesByUserId() const;
    void SetLastExitReason(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetService);
    inline bool IsUIExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    inline bool IsCacheExtensionAbilityType(const std::shared_ptr<AbilityRecord> &abilityRecord);
    inline bool CheckUIExtensionAbilityLoaded(const AbilityRequest &abilityRequest);
    inline bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<AbilityRecord> &abilityRecord);
    inline void RemoveUIExtensionAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);
    inline void AddUIExtensionAbilityRecordToTerminatedList(const std::shared_ptr<AbilityRecord> &abilityRecord);
    inline bool IsCallerValid(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<AbilityRecord> &extensionRecord, bool &isLoaded);
    int32_t GetOrCreateTargetServiceRecord(
        const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
        std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility);
    void HandleNotifyAssertFaultDialogDied(const std::shared_ptr<AbilityRecord> &abilityRecord);
    EventInfo BuildEventInfo(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void UpdateUIExtensionInfo(const std::shared_ptr<AbilityRecord> &abilityRecord);
    std::string GenerateBundleName(const AbilityRequest &abilityRequest) const;

    bool AddToServiceMap(const std::string &key, std::shared_ptr<AbilityRecord> abilityRecord);
    ServiceMapType GetServiceMap();

    void AddConnectObjectToMap(sptr<IRemoteObject> connectObject, const ConnectListType &connectRecordList,
        bool updateOnly);

    void KeepAbilityAlive(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);
    void ProcessEliminateAbilityRecord(std::shared_ptr<AbilityRecord> eliminateRecord);
    std::string GetServiceKey(const std::shared_ptr<AbilityRecord> &service);

    int32_t ReportXiaoYiToRSSIfNeeded(const AppExecFwk::AbilityInfo &abilityInfo);
    int32_t ReportAbilitStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);

private:
    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    const std::string TASK_ON_ABILITY_DIED = "OnAbilityDiedTask";

    ffrt::mutex serialMutex_;

    std::mutex connectMapMutex_;
    ConnectMapType connectMap_;

    ffrt::mutex serviceMapMutex_;
    ServiceMapType serviceMap_;
    std::list<std::shared_ptr<AbilityRecord>> terminatingExtensionList_;

    std::mutex recipientMapMutex_;
    RecipientMapType recipientMap_;

    std::mutex uiExtRecipientMapMutex_;
    RecipientMapType uiExtRecipientMap_;

    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
    int userId_;
    std::vector<AbilityRequest> restartResidentTaskList_;

    std::mutex startServiceReqListLock_;
    std::unordered_map<std::string, std::shared_ptr<std::list<AbilityRequest>>> startServiceReqList_;

    std::mutex uiExtensionMapMutex_;
    UIExtensionMapType uiExtensionMap_;

    std::mutex windowExtensionMapMutex_;
    WindowExtensionMapType windowExtensionMap_;

    std::unique_ptr<UIExtensionAbilityConnectManager> uiExtensionAbilityRecordMgr_ = nullptr;
    uint32_t sceneBoardTokenId_ = 0;

    DISALLOW_COPY_AND_MOVE(AbilityConnectManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
