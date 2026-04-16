/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <atomic>
#include <deque>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include "cpp/mutex.h"

#include "ability_cache_manager.h"
#include "ability_connect_callback_interface.h"
#include "caller_info.h"
#include "task_handler_wrap.h"
#include "event_handler_wrap.h"
#include "base_extension_record.h"
#include "event_report.h"
#include "extension_config.h"
#include "extension_running_info.h"
#include "connection_record.h"
#include "element_name.h"
#include "res_sched_util.h"
#include "ui_extension/ui_extension_ability_connect_info.h"
#include "want.h"
#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::AbilityType;
using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;
/**
 * @class AbilityConnectManager
 * AbilityConnectManager provides a facility for managing service ability connection.
 */
class AbilityConnectManager : public std::enable_shared_from_this<AbilityConnectManager> {
public:
    using ConnectMapType = std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<ConnectionRecord>>>;
    using ServiceMapType = std::map<std::string, std::shared_ptr<BaseExtensionRecord>>;
    using ConnectListType = std::list<std::shared_ptr<ConnectionRecord>>;
    using RecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;
    using WindowExtMapValType = std::pair<uint32_t, sptr<SessionInfo>>;
    using WindowExtensionMapType = std::map<sptr<IRemoteObject>, WindowExtMapValType>;

    explicit AbilityConnectManager(int userId);
    virtual ~AbilityConnectManager();

    struct LoadAbilityContext {
        std::shared_ptr<AbilityRuntime::LoadParam> loadParam;
        std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
        std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo;
        std::shared_ptr<AAFwk::Want> want;
    };

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartAbility(const AbilityRequest &abilityRequest);

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
     * @param indirectCallerInfo, Indirect caller information.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
        std::shared_ptr<IndirectCallerInfo> indirectCallerInfo = nullptr);

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * SuspendExtensionAbilityLocked, suspend session with callback.
     *
     * @param connect, Callback used to notify caller the result of suspend.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SuspendExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * ResumeExtensionAbilityLocked, resume session with callback.
     *
     * @param connect, Callback used to notify caller the result of resume.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ResumeExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * AttachAbilityThreadLocked, ability call this interface after loaded.
     *
     * @param scheduler, the interface handler of kit ability.
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) {}

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
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig);

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

    std::shared_ptr<BaseExtensionRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token);
    std::shared_ptr<BaseExtensionRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token);
    std::shared_ptr<BaseExtensionRecord> GetExtensionByIdFromServiceMap(const int64_t &abilityRecordId);
    std::shared_ptr<BaseExtensionRecord> GetExtensionByIdFromTerminatingMap(const int64_t &abilityRecordId);
    std::shared_ptr<BaseExtensionRecord> GetUIExtensionBySessionFromServiceMap(const sptr<SessionInfo> &sessionInfo);
    ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    static void GetExtensionRunningInfos(ServiceMapType &serviceMapBack, int upperLimit,
        std::vector<ExtensionRunningInfo> &info, const int32_t userId, bool isPerm);

    static void GetExtensionRunningInfo(std::shared_ptr<BaseExtensionRecord> &abilityRecord, const int32_t userId,
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

    void OnLoadAbilityFailed(std::shared_ptr<BaseExtensionRecord> abilityRecord);

    /**
     * OnAbilityDied.
     *
     * @param abilityRecord, service ability record.
     */
    void OnAbilityDied(const std::shared_ptr<BaseExtensionRecord> abilityRecord);

    /**
     * DisconnectBeforeCleanup.
     *
     */
    void DisconnectBeforeCleanup();

    static void DumpState(
        ServiceMapType &serviceMapBack, std::vector<std::string> &info, bool isClient, const std::string &args = "");

    static void DumpStateByUri(ServiceMapType &serviceMapBack, std::vector<std::string> &info, bool isClient,
        const std::string &args, std::vector<std::string> &params);

    void PauseExtensions();

    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false);

    void CommandAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);

    bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken);

    void HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid);

    void TerminateAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void RemoveLauncherDeathRecipient();

    void CloseAssertDialog(const std::string &assertSessionId);

    void SignRestartAppFlag(int32_t uid, const std::string &instanceKey);
    void SignRestartProcess(int32_t pid);

    void UninstallApp(const std::string &bundleName, int32_t uid);

    int32_t UpdateKeepAliveEnableState(const std::string &bundleName, const std::string &moduleName,
        const std::string &mainElement, bool updateEnable);

    /**
     * Handle extension disconnect task.
     */
    void HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * GetServiceRecordByAbilityRequest.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns BaseExtensionRecord shared_ptr.
     */
    std::shared_ptr<BaseExtensionRecord> GetServiceRecordByAbilityRequest(const AbilityRequest &abilityRequest);

    bool HasRequestIdInLoadAbilityQueue(int32_t requestId);
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId);
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId);
    void StartSpecifiedProcess(const LoadAbilityContext &context,
        const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    ServiceMapType GetServiceMap();
    uint32_t GetSceneBoardTokenId() const
    {
        return sceneBoardTokenId_;
    }
protected:

    /**
    * Move ability record to terminating map.
    *
    * @param abilityRecord The ability record to move.
    */
    void MoveToTerminatingMap(const std::shared_ptr<BaseExtensionRecord>& abilityRecord);

    /**
    * Check if ability needs to be kept alive.
    *
    * @param abilityRecord The ability record to check.
    * @return Returns true if ability needs keep alive, false otherwise.
    */
    bool IsAbilityNeedKeepAlive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    /**
    * Check if extension ability should be cached by ability info.
    *
    * @param abilityInfo The ability information.
    * @return Returns true if should cache, false otherwise.
    */
    bool IsCacheExtensionAbilityByInfo(const AppExecFwk::AbilityInfo &abilityInfo);

    /**
    * Check if ability is special type.
    *
    * @param abilityInfo The ability information.
    * @return Returns true if special ability, false otherwise.
    */
    bool IsSpecialAbility(const AppExecFwk::AbilityInfo &abilityInfo);

    /**
    * Report XiaoYi information to RSS if needed.
    *
    * @param abilityInfo The ability information.
    * @return Returns error code.
    */
    int32_t ReportXiaoYiToRSSIfNeeded(const AppExecFwk::AbilityInfo &abilityInfo);

    /**
    * Report ability start information to RSS.
    *
    * @param abilityInfo The ability information.
    * @return Returns error code.
    */
    int32_t ReportAbilityStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);

    /**
    * Report event to RSS.
    *
    * @param abilityInfo The ability information.
    * @param abilityRecord The ability record.
    * @param callerToken The caller token.
    */
    void ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo,
        const std::shared_ptr<BaseExtensionRecord> abilityRecord, sptr<IRemoteObject> callerToken);

    /**
    * Generate bundle name from ability request.
    *
    * @param abilityRequest The ability request.
    * @return Returns generated bundle name.
    */
    static std::string GenerateBundleName(const AbilityRequest &abilityRequest);

    /**
    * Call EnqueueStartServiceReq internally.
    *
    * @param abilityRequest The ability request.
    * @param uri The URI for the service.
    */
    void CallEnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &uri)
    {
        EnqueueStartServiceReq(abilityRequest, uri);
    }

    /**
    * Call AddToServiceMap internally.
    *
    * @param key The service key.
    * @param abilityRecord The ability record to add.
    */
    void CallAddToServiceMap(const std::string &key, std::shared_ptr<BaseExtensionRecord> abilityRecord)
    {
        AddToServiceMap(key, abilityRecord);
    }

    /**
    * Remove service from map safely.
    *
    * @param serviceKey The service key to remove.
    */
    void RemoveServiceFromMapSafe(const std::string &serviceKey);

    /**
    * Load ability with optional callback.
    *
    * @param abilityRecord The ability record to load.
    * @param updateRecordCallback Callback to update record after loading.
    * @param isPreloadUIExtension Whether loading for UI Extension preload.
    * @param timeout, timeout multiply for ability loading stage, not work on asan
    */
    void LoadAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        std::function<void(const std::shared_ptr<BaseExtensionRecord> &)> updateRecordCallback = nullptr,
        bool isPreloadUIExtension = false, int32_t loadTimeout = 0);

    /**
     * RemoveServiceAbility.
     *
     * @param service, the ptr of the ability record.
     */
    void RemoveServiceAbility(const std::shared_ptr<BaseExtensionRecord> &service);

    int UpdateStateAndCompleteDisconnect(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const std::shared_ptr<ConnectionRecord> &connect, const sptr<IRemoteObject> &token);

    void HandleConnectRecordOnAbilityDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void HandleAfterServiceRemoved(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, bool isRemove);

    void HandlePostConnectTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int connectRecordId, int64_t recordId);

    bool ReportConnectEventToRss(LoadingStage stage,
        std::shared_ptr<BaseExtensionRecord> abilityRecord,
        std::shared_ptr<ConnectionRecord> connectRecord,
        int64_t timeDuration);

    void PrintTimeOutLog(const std::shared_ptr<BaseExtensionRecord> &ability, uint32_t msgId, bool isHalf = false);
    /**
     * When a service is under starting, enque the request and handle it after the service starting completes
     */
    void EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri = "");
    /**
     * After the service starting completes, complete the request list
     */
    void CompleteStartServiceReq(const std::string &serviceUri);

    bool IsCacheExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void HandleActiveAbility(std::shared_ptr<BaseExtensionRecord> &targetService,
        std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * CommandAbility. Schedule command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CommandAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void ProcessPreload(const std::shared_ptr<BaseExtensionRecord> &record) const;
protected:
    /**
    * Start ability with locking mechanism.
    *
    * @param abilityRequest The request for starting ability.
    * @return Returns error code.
    */
    virtual int32_t StartAbilityLocked(const AbilityRequest &abilityRequest);

    /**
     * TerminateAbilityLocked with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @param resultCode, the result code of service type's ability to terminate.
     * @param resultWant, the result want for service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbilityLocked(const sptr<IRemoteObject> &token);

    /**
    * Attach ability thread internally.
    *
    * @param scheduler The ability scheduler interface.
    * @param token The remote object token.
    * @param abilityRecord The ability record to attach.
    * @return Returns error code.
    */
    virtual int AttachAbilityThreadInner(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    virtual int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest,
        std::shared_ptr<BaseExtensionRecord> &targetService, bool &isLoadedAbility)
    {
        return 0;
    }

    virtual void HandleStartTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    virtual void HandleForegroundTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    virtual void HandleStopTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    virtual void CleanActivatingTimeoutAbilityInner(std::shared_ptr<BaseExtensionRecord> abilityRecord);

    virtual void HandleAbilityDiedTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    /**
    * Handle loading ability or starting specified process.
    *
    * @param loadParam The load parameters.
    * @param abilityRecord The ability record to load.
    */
    virtual void HandleLoadAbilityOrStartSpecifiedProcess(
        const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    /**
    * Terminate ability internally.
    *
    * @param token The token of the ability to terminate.
    * @return Returns error code.
    */
    virtual int TerminateAbilityInner(const sptr<IRemoteObject> &token);

    /**
     * TerminateDone.
     *
     * @param abilityRecord, the ptr of the ability to terminate.
     */
    virtual void TerminateDone(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    virtual bool HandleExtensionAbilityRemove(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    virtual void PostLoadTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int32_t loadTimeout);

    virtual int DispatchForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<IRemoteObject> &token);
    virtual int DispatchBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<IRemoteObject> &token);

    /**
     * dispatch service ability life cycle .
     *
     * @param abilityRecord.
     * @param state.
     */
    virtual int DispatchInactive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int state,
        const sptr<IRemoteObject> &token);

    virtual int CheckAbilityStateForDisconnect(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    virtual int CleanupConnectionAndTerminateIfNeeded(std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    virtual void CompleteForegroundInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    virtual int32_t ConnectAbilityLockedInner(bool isLoadedAbility,
        std::shared_ptr<BaseExtensionRecord>& targetService, const AbilityRequest& abilityRequest,
        std::shared_ptr<ConnectionRecord>& connectRecord);
    virtual void TerminateOrCacheAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    virtual void HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo);
    virtual void CompleteBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
protected:

    /**
    * Remove UI Extension by session info token.
    *
    * @param token The session token to identify the UI Extension.
    * @return Returns ERR_OK on success, others on failure.
    */
    virtual int RemoveUIExtensionBySessionInfoToken(sptr<IRemoteObject> token)
    {
        return 0;
    }

    /**
    * Handle UI Extension death event.
    *
    * @param abilityRecord The ability record of the died UI Extension.
    */
    virtual void HandleUIExtensionDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) {}

    /**
    * Remove UI Extension ability record from management.
    *
    * @param abilityRecord The ability record to remove.
    */
    virtual void RemoveUIExtensionAbilityRecord(const std::shared_ptr<BaseExtensionRecord> &abilityRecord) {}

    /**
    * Remove UI Extension window death recipient.
    *
    * @param session The session object to remove death recipient for.
    */
    virtual void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session) {}

    /**
    * Background the ability window for UI Extension.
    *
    * @param abilityRecord The ability record to background.
    * @param sessionInfo The session information.
    */
    virtual void DoBackgroundAbilityWindow(
        const std::shared_ptr<BaseExtensionRecord> &abilityRecord, const sptr<SessionInfo> &sessionInfo) {}

    /**
    * Check if UI Extension ability session exists.
    *
    * @param abilityRecord The ability record to check.
    * @return Returns true if session exists, false otherwise.
    */
    virtual bool CheckUIExtensionAbilitySessionExist(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
    {
        return true;
    }

    /**
    * Unregister preload UI Extension host client.
    *
    * @param callerPid The caller process ID.
    * @return Returns ERR_OK on success, others on failure.
    */
    virtual int32_t UnRegisterPreloadUIExtensionHostClient(int32_t callerPid)
    {
        return 0;
    }

    /**
    * Add preload UI Extension record.
    *
    * @param abilityRecord The preload ability record to add.
    * @return Returns 0 on success, -1 on failure.
    */
    virtual int32_t AddPreloadUIExtensionRecord(const std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord)
    {
        return 0;
    }
protected:
    int userId_;
    ServiceMapType serviceMap_;
    ffrt::mutex serviceMapMutex_;
    ffrt::mutex serialMutex_;
    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;

private:
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
     * ConnectAbility.Schedule connect ability for per-connection-type extensions
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectPerConnectionTypeExtension(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        int connectRecordId, const Want &want);

    /**
     * ConnectAbility.Schedule Resume Connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ResumeConnectAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    /**
     * CompleteCommandAbility. complete command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CompleteCommandAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);

    /**
     * GetServiceRecordByElementName.
     *
     * @param element, service ability's element.
     * @return Returns BaseExtensionRecord shared_ptr.
     */
    std::shared_ptr<BaseExtensionRecord> GetServiceRecordByElementName(const std::string &element);

    /**
     * GetAbilityConnectedRecordFromRecordList.
     *
     * @param targetService, the ptr of the connected ability.
     * @param connectRecordList, connect record list.
     * @return Returns ConnectionRecord shared_ptr.
     */
    std::shared_ptr<ConnectionRecord> GetAbilityConnectedRecordFromRecordList(
        const std::shared_ptr<BaseExtensionRecord> &targetService,
        std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList);

    int DispatchTerminate(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, const sptr<IRemoteObject> &token);

    void HandleStartTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    void HandleStopTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    void HandleTerminateDisconnectTask(const ConnectListType& connectlist);
    void HandleCommandTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    void HandleCommandWindowTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);
    void HandleForegroundTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    void HandleConnectTimeoutTask(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    void HandleRestartResidentTask(const AbilityRequest &abilityRequest);

    /**
     * RemoveConnectionRecordFromMap.
     *
     * @param connect, the ptr of the connect record.
     */
    void RemoveConnectionRecordFromMap(std::shared_ptr<ConnectionRecord> connect);

    /**
    * GetOrCreateServiceRecord.
    *
    * @param abilityRequest, Special want for service type's ability.
    * @param isCreatedByConnect, whether is created by connect ability mode.
    * @param targetAbilityRecord, the target service ability record.
    * @param isLoadedAbility, whether the target ability has been loaded.
    */
    void GetOrCreateServiceRecord(const AbilityRequest &abilityRequest, const bool isCreatedByConnect,
        std::shared_ptr<BaseExtensionRecord> &targetAbilityRecord, bool &isLoadedAbility);
    void SetServiceAfterNewCreate(const AbilityRequest &abilityRequest, BaseExtensionRecord &targetService);

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
    void HandleAbilityDiedTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void RestartAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int32_t currentUserId);

    /**
     * PostTimeOutTask.
     *
     * @param abilityRecord, ability.
     * @param messageId, message id.
     */
    void PostTimeOutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, uint32_t messageId);
    void PostTimeOutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int connectRecordId,
        uint32_t messageId);
    int32_t GetLoadTimeout(int32_t loadTimeout);

    void CompleteForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);

    void PostRestartResidentTask(const AbilityRequest &abilityRequest);

    void HandleInactiveTimeout(const std::shared_ptr<BaseExtensionRecord> &ability);
    void CleanActivatingTimeoutAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord);

    /**
     * Post an extension's disconnect task, auto disconnect when extension connected timeout.
     */
    void PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Remove the extension's disconnect task.
     */
    void RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

private:
    bool IsNeedToRestart(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
        const std::string &bundleName, const std::string &abilityName);
    void TerminateRecord(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    int DisconnectRecordNormal(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord,
        bool callerDied) const;
    void DisconnectRecordForce(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord);
    bool IsLauncher(std::shared_ptr<BaseExtensionRecord> serviceExtension) const;
    void KillProcessesByUserId() const;
    void HandleNotifyAssertFaultDialogDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    EventInfo BuildEventInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    bool AddToServiceMap(const std::string &key, std::shared_ptr<BaseExtensionRecord> abilityRecord);

    void AddConnectObjectToMap(sptr<IRemoteObject> connectObject, const ConnectListType &connectRecordList,
        bool updateOnly);

    void KeepAbilityAlive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord);
    void ProcessEliminateAbilityRecord(std::shared_ptr<BaseExtensionRecord> eliminateRecord);
    static std::string GetServiceKey(const std::shared_ptr<BaseExtensionRecord> &service);
    static std::string GetServiceKey(const AbilityRequest &abilityRequest);

    void SetExtensionLoadParam(AbilityRuntime::LoadParam &loadParam,
        std::shared_ptr<BaseExtensionRecord> abilityRecord);
    bool IsStrictMode(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    bool NeedExtensionControl(std::shared_ptr<BaseExtensionRecord> abilityRecord);
    bool GetTimeoutMsgContent(uint32_t msgId, std::string &msgContent, int &typeId);

    void HandleConnectionCountIncrement(int32_t pid, const std::string &callerBundleName,
        const std::string &targetName);
    void DecrementConnectionCountAndCleanup(int32_t pid);

private:
    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    const std::string TASK_ON_ABILITY_DIED = "OnAbilityDiedTask";

    ConnectMapType connectMap_;
    RecipientMapType recipientMap_;
    WindowExtensionMapType windowExtensionMap_;
    std::map<int32_t, int32_t> callerPidConnectionCountMap_;

    std::list<std::shared_ptr<BaseExtensionRecord>> terminatingExtensionList_;
    std::vector<AbilityRequest> restartResidentTaskList_;
    std::unordered_map<std::string, std::shared_ptr<std::list<AbilityRequest>>> startServiceReqList_;

    std::mutex connectMapMutex_;
    std::mutex recipientMapMutex_;
    std::mutex windowExtensionMapMutex_;
    std::mutex startServiceReqListLock_;
    std::mutex loadAbilityQueueLock_;
    std::mutex callerPidConnectionCountMapMutex_;
    std::deque<std::map<int32_t, LoadAbilityContext>> loadAbilityQueue_;
    std::vector<int32_t> thresholds_ = {50, 100, 200, 500};
    std::atomic_uint32_t sceneBoardTokenId_ = 0;

    DISALLOW_COPY_AND_MOVE(AbilityConnectManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
