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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H

#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <vector>
#include <utility>
#include "cpp/mutex.h"
#include "cpp/condition_variable.h"

#include "ability_connect_callback_interface.h"
#include "ability_info.h"
#include "ability_start_setting.h"
#include "ability_state.h"
#include "ability_token_stub.h"
#include "app_scheduler.h"
#include "application_info.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "call_container.h"
#include "exit_reason.h"
#include "ipc_skeleton.h"
#include "lifecycle_deal.h"
#include "lifecycle_state_info.h"
#include "session_info.h"
#include "ui_extension_window_command.h"
#include "uri.h"
#include "want.h"
#include "window_config.h"
#ifdef SUPPORT_GRAPHICS
#include "ability_window_configuration.h"
#include "resource_manager.h"
#include "start_options.h"
#include "window_manager_service_handler.h"
#endif

namespace OHOS {
namespace AAFwk {
using Closure = std::function<void()>;

class AbilityRecord;
class ConnectionRecord;
class CallContainer;

constexpr const char* ABILITY_TOKEN_NAME = "AbilityToken";
constexpr const char* LAUNCHER_BUNDLE_NAME = "com.ohos.launcher";

/**
 * @class Token
 * Token is identification of ability and used to interact with kit and wms.
 */
class Token : public AbilityTokenStub {
public:
    explicit Token(std::weak_ptr<AbilityRecord> abilityRecord);
    virtual ~Token();

    std::shared_ptr<AbilityRecord> GetAbilityRecord() const;
    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token);

private:
    std::weak_ptr<AbilityRecord> abilityRecord_;  // ability of this token
};

/**
 * @class AbilityResult
 * Record requestCode of for-result start mode and result.
 */
class AbilityResult {
public:
    AbilityResult() = default;
    AbilityResult(int requestCode, int resultCode, const Want &resultWant)
        : requestCode_(requestCode), resultCode_(resultCode), resultWant_(resultWant)
    {}
    virtual ~AbilityResult()
    {}

    int requestCode_ = -1;  // requestCode of for-result start mode
    int resultCode_ = -1;   // resultCode of for-result start mode
    Want resultWant_;       // for-result start mode ability will send the result to caller
};

/**
 * @class SystemAbilityCallerRecord
 * Record system caller ability of for-result start mode and result.
 */
class SystemAbilityCallerRecord {
public:
    SystemAbilityCallerRecord(std::string &srcAbilityId, const sptr<IRemoteObject> &callerToken)
        : srcAbilityId_(srcAbilityId), callerToken_(callerToken)
    {}
    virtual ~SystemAbilityCallerRecord()
    {}

    std::string GetSrcAbilityId()
    {
        return srcAbilityId_;
    }
    const sptr<IRemoteObject> GetCallerToken()
    {
        return callerToken_;
    }
    void SetResult(Want &want, int resultCode)
    {
        resultWant_ = want;
        resultCode_ = resultCode;
    }
    Want &GetResultWant()
    {
        return resultWant_;
    }
    int &GetResultCode()
    {
        return resultCode_;
    }
    /**
     * Set result to system ability.
     *
     */
    void SetResultToSystemAbility(std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord,
        Want &resultWant, int resultCode);
    /**
     * Send result to system ability.
     *
     */
    void SendResultToSystemAbility(int requestCode,
        const std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord,
        int32_t callerUid, uint32_t accessToken, bool schedulerdied);

private:
    std::string srcAbilityId_;
    sptr<IRemoteObject> callerToken_;
    Want resultWant_;
    int resultCode_ = -1;
};

/**
 * @struct CallerAbilityInfo
 * caller ability info.
 */
struct CallerAbilityInfo {
public:
    std::string callerBundleName;
    std::string callerAbilityName;
    int32_t callerTokenId = 0;
    int32_t callerUid = 0;
    int32_t callerPid = 0;
};

/**
 * @class CallerRecord
 * Record caller ability of for-result start mode and result.
 */
class CallerRecord {
public:
    CallerRecord() = default;
    CallerRecord(int requestCode, std::weak_ptr<AbilityRecord> caller);
    CallerRecord(int requestCode, std::shared_ptr<SystemAbilityCallerRecord> saCaller) : requestCode_(requestCode),
        saCaller_(saCaller)
    {}
    virtual ~CallerRecord()
    {}

    int GetRequestCode()
    {
        return requestCode_;
    }
    std::shared_ptr<AbilityRecord> GetCaller()
    {
        return caller_.lock();
    }
    std::shared_ptr<SystemAbilityCallerRecord> GetSaCaller()
    {
        return saCaller_;
    }
    std::shared_ptr<CallerAbilityInfo> GetCallerInfo()
    {
        return callerInfo_;
    }

private:
    int requestCode_ = -1;  // requestCode of for-result start mode
    std::weak_ptr<AbilityRecord> caller_;
    std::shared_ptr<SystemAbilityCallerRecord> saCaller_ = nullptr;
    std::shared_ptr<CallerAbilityInfo> callerInfo_ = nullptr;
};

/**
 * @class AbilityRequest
 * Wrap parameters of starting ability.
 */
enum AbilityCallType {
    INVALID_TYPE = 0,
    CALL_REQUEST_TYPE,
    START_OPTIONS_TYPE,
    START_SETTINGS_TYPE,
    START_EXTENSION_TYPE,
};

enum CollaboratorType {
    DEFAULT_TYPE = 0,
    RESERVE_TYPE,
    OTHERS_TYPE
};

struct AbilityRequest {
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo appInfo;
    int32_t uid = 0;
    int requestCode = -1;
    bool restart = false;
    int32_t restartCount = -1;
    int64_t restartTime = 0;
    bool startRecent = false;
    int32_t collaboratorType = CollaboratorType::DEFAULT_TYPE;

    // call ability
    int callerUid = -1;
    AbilityCallType callType = AbilityCallType::INVALID_TYPE;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> asCallerSourceToken = nullptr;
    uint32_t callerAccessTokenId = -1;
    sptr<IAbilityConnection> connect = nullptr;

    std::shared_ptr<AbilityStartSetting> startSetting = nullptr;
    std::shared_ptr<ProcessOptions> processOptions = nullptr;
    std::string specifiedFlag;
    int32_t userId = -1;
    bool callSpecifiedFlagTimeout = false;
    sptr<IRemoteObject> abilityInfoCallback = nullptr;

    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    AppExecFwk::ExtensionProcessMode extensionProcessMode = AppExecFwk::ExtensionProcessMode::UNDEFINED;

    sptr<SessionInfo> sessionInfo;
    uint32_t specifyTokenId = 0;
    bool uriReservedFlag = false;
    std::string reservedBundleName;

    std::pair<bool, LaunchReason> IsContinuation() const
    {
        auto flags = want.GetFlags();
        if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
            return {true, LaunchReason::LAUNCHREASON_CONTINUATION};
        }
        if ((flags & Want::FLAG_ABILITY_PREPARE_CONTINUATION) == Want::FLAG_ABILITY_PREPARE_CONTINUATION) {
            return {true, LaunchReason::LAUNCHREASON_PREPARE_CONTINUATION};
        }
        return {false, LaunchReason::LAUNCHREASON_UNKNOWN};
    }

    bool IsAcquireShareData() const
    {
        return want.GetBoolParam(Want::PARAM_ABILITY_ACQUIRE_SHARE_DATA, false);
    }

    bool IsAppRecovery() const
    {
        return want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false);
    }

    bool IsCallType(const AbilityCallType & type) const
    {
        return (callType == type);
    }

    void Dump(std::vector<std::string> &state)
    {
        std::string dumpInfo = "      want [" + want.ToUri() + "]";
        state.push_back(dumpInfo);
        dumpInfo = "      app name [" + abilityInfo.applicationName + "]";
        state.push_back(dumpInfo);
        dumpInfo = "      main name [" + abilityInfo.name + "]";
        state.push_back(dumpInfo);
        dumpInfo = "      request code [" + std::to_string(requestCode) + "]";
        state.push_back(dumpInfo);
    }

    void Voluation(const Want &srcWant, int srcRequestCode,
        const sptr<IRemoteObject> &srcCallerToken, const std::shared_ptr<AbilityStartSetting> srcStartSetting = nullptr,
        int srcCallerUid = -1)
    {
        want = srcWant;
        requestCode = srcRequestCode;
        callerToken = srcCallerToken;
        startSetting = srcStartSetting;
        callerUid = srcCallerUid == -1 ? IPCSkeleton::GetCallingUid() : srcCallerUid;
    }
};

// new version
enum ResolveResultType {
    OK_NO_REMOTE_OBJ = 0,
    OK_HAS_REMOTE_OBJ,
    NG_INNER_ERROR,
};

enum class AbilityWindowState {
    FOREGROUND = 0,
    BACKGROUND,
    TERMINATE,
    FOREGROUNDING,
    BACKGROUNDING,
    TERMINATING
};

enum class AbilityVisibilityState {
    INITIAL = 0,
    FOREGROUND_HIDE,
    FOREGROUND_SHOW,
    UNSPECIFIED,
};

/**
 * @class AbilityRecord
 * AbilityRecord records ability info and states and used to schedule ability life.
 */
class AbilityRecord : public std::enable_shared_from_this<AbilityRecord> {
public:
    AbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode = -1);

    virtual ~AbilityRecord();

    /**
     * CreateAbilityRecord.
     *
     * @param abilityRequest,create ability record.
     * @return Returns ability record ptr.
     */
    static std::shared_ptr<AbilityRecord> CreateAbilityRecord(const AbilityRequest &abilityRequest);

    /**
     * Init ability record.
     *
     * @return Returns true on success, others on failure.
     */
    bool Init();

    /**
     * load UI ability.
     *
     */
    void LoadUIAbility();

    /**
     * load ability.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int LoadAbility();

    /**
     * foreground the ability.
     *
     */
    void ForegroundAbility(uint32_t sceneFlag = 0);
    void ForegroundAbility(const Closure &task, uint32_t sceneFlag = 0);

    /**
     * process request of foregrounding the ability.
     *
     */
    void ProcessForegroundAbility(uint32_t tokenId, uint32_t sceneFlag = 0);

     /**
     * post foreground timeout task for ui ability.
     *
     */
    void PostForegroundTimeoutTask();

    /**
     * move the ability to back ground.
     *
     * @param task timeout task.
     */
    void BackgroundAbility(const Closure &task);

    /**
     * prepare terminate ability.
     *
     * @return Returns true on stop terminating; returns false on terminate.
     */
    bool PrepareTerminateAbility();

    /**
     * terminate ability.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility();

    /**
     * get ability's info.
     *
     * @return ability info.
     */
    const AppExecFwk::AbilityInfo &GetAbilityInfo() const;

    /**
     * get application's info.
     *
     * @return application info.
     */
    const AppExecFwk::ApplicationInfo &GetApplicationInfo() const;

    /**
     * set ability's state.
     *
     * @param state, ability's state.
     */
    void SetAbilityState(AbilityState state);

    bool GetAbilityForegroundingFlag() const;

    void SetAbilityForegroundingFlag();

    /**
     * get ability's state.
     *
     * @return ability state.
     */
    AbilityState GetAbilityState() const;

    /**
     * get ability's windowconfig.
     *
     * @return ability windowconfig.
     */
    WindowConfig GetAbilityWindowConfig() const;

    bool IsForeground() const;

    AbilityVisibilityState GetAbilityVisibilityState() const;
    void SetAbilityVisibilityState(AbilityVisibilityState state);

    void UpdateAbilityVisibilityState();

    /**
     * set ability scheduler for accessing ability thread.
     *
     * @param scheduler , ability scheduler.
     */
    void SetScheduler(const sptr<IAbilityScheduler> &scheduler);

    inline sptr<IAbilityScheduler> GetScheduler() const
    {
        return scheduler_;
    }

    sptr<SessionInfo> GetSessionInfo() const;

    /**
     * get ability's token.
     *
     * @return ability's token.
     */
    sptr<Token> GetToken() const;

    /**
     * set ability's previous ability record.
     *
     * @param abilityRecord , previous ability record
     */
    void SetPreAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * get ability's previous ability record.
     *
     * @return previous ability record
     */
    std::shared_ptr<AbilityRecord> GetPreAbilityRecord() const;

    /**
     * set ability's next ability record.
     *
     * @param abilityRecord , next ability record
     */
    void SetNextAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * get ability's previous ability record.
     *
     * @return previous ability record
     */
    std::shared_ptr<AbilityRecord> GetNextAbilityRecord() const;

    /**
     * check whether the ability is ready.
     *
     * @return true : ready ,false: not ready
     */
    bool IsReady() const;

    void UpdateRecoveryInfo(bool hasRecoverInfo);

    bool GetRecoveryInfo();

#ifdef SUPPORT_SCREEN
    /**
     * check whether the ability 's window is attached.
     *
     * @return true : attached ,false: not attached
     */
    bool IsWindowAttached() const;

    inline bool IsStartingWindow() const
    {
        return isStartingWindow_;
    }

    inline void SetStartingWindow(bool isStartingWindow)
    {
        isStartingWindow_ = isStartingWindow;
    }

    void PostCancelStartingWindowHotTask();

    /**
     * process request of foregrounding the ability.
     *
     */
    void ProcessForegroundAbility(bool isRecent, const AbilityRequest &abilityRequest,
        std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<AbilityRecord> &callerAbility,
        uint32_t sceneFlag = 0);

    void ProcessForegroundAbility(const std::shared_ptr<AbilityRecord> &callerAbility, bool needExit = true,
        uint32_t sceneFlag = 0);
    void NotifyAnimationFromTerminatingAbility() const;
    void NotifyAnimationFromMinimizeAbility(bool& animaEnabled);

    bool ReportAtomicServiceDrawnCompleteEvent();
    void SetCompleteFirstFrameDrawing(const bool flag);
    bool IsCompleteFirstFrameDrawing() const;
    bool GetColdStartFlag();
    void SetColdStartFlag(bool isColdStart);
#endif

    bool GrantUriPermissionForServiceExtension();

    /**
     * check whether the ability is launcher.
     *
     * @return true : lanucher ,false: not lanucher
     */
    bool IsLauncherAbility() const;

    /**
     * check whether the ability is terminating.
     *
     * @return true : yes ,false: not
     */
    bool IsTerminating() const;

    /**
     * set the ability is terminating.
     *
     */
    void SetTerminatingState();

    /**
     * set the ability is new want flag.
     *
     * @return isNewWant
     */
    void SetIsNewWant(bool isNewWant);

    /**
     * check whether the ability is new want flag.
     *
     * @return true : yes ,false: not
     */
    bool IsNewWant() const;

    /**
     * check whether the ability is created by connect ability mode.
     *
     * @return true : yes ,false: not
     */
    bool IsCreateByConnect() const;

    /**
     * set the ability is created by connect ability mode.
     *
     */
    void SetCreateByConnectMode(bool isCreatedByConnect = true);

    /**
     * active the ability.
     *
     */
    virtual void Activate();

    /**
     * inactive the ability.
     *
     */
    virtual void Inactivate();

    /**
     * terminate the ability.
     *
     */
    void Terminate(const Closure &task);

    /**
     * connect the ability.
     *
     */
    void ConnectAbility();

    /**
     * connect the ability.
     *
     */
    void ConnectUIServiceExtAbility(const Want &want);

    /**
     * disconnect the ability.
     *
     */
    void DisconnectAbility();

    /**
     * disconnect the ability with want
     *
     */
    void DisconnectUIServiceExtAbility(const Want &want);

    /**
     * Command the ability.
     *
     */
    void CommandAbility();

    void CommandAbilityWindow(const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);

    /**
     * save ability state.
     *
     */
    void SaveAbilityState();
    void SaveAbilityState(const PacMap &inState);
    void SaveAbilityWindowConfig(const WindowConfig &windowConfig);

    /**
     * restore ability state.
     *
     */
    void RestoreAbilityState();

    /**
     * notify top active ability updated.
     *
     */
    void TopActiveAbilityChanged(bool flag);

    /**
     * set the want for start ability.
     *
     */
    void SetWant(const Want &want);

    /**
     * get the want for start ability.
     *
     */
    Want GetWant() const;

    /**
     * remove specified wantParam for start ability.
     *
     */
    void RemoveSpecifiedWantParam(const std::string &key);

    /**
     * get request code of the ability to start.
     *
     */
    int GetRequestCode() const;

    /**
     * set the result object of the ability which one need to be terminated.
     *
     */
    void SetResult(const std::shared_ptr<AbilityResult> &result);

    /**
     * get the result object of the ability which one need to be terminated.
     *
     */
    std::shared_ptr<AbilityResult> GetResult() const;

    /**
     * send result object to caller ability thread.
     *
     */
    void SendResult(bool isSandboxApp, uint32_t tokeId);

    /**
     * send result object to caller ability thread for sandbox app file saving.
     */
    void SendSandboxSavefileResult(const Want &want, int resultCode, int requestCode);

    /**
     * send result object to caller ability.
     *
     */
    void SendResultToCallers(bool schedulerdied = false);

    /**
     * save result object to caller ability.
     *
     */
    void SaveResultToCallers(const int resultCode, const Want *resultWant);

    /**
     * save result to caller ability.
     *
     */
    void SaveResult(int resultCode, const Want *resultWant, std::shared_ptr<CallerRecord> caller);

    bool NeedConnectAfterCommand();

    /**
     * add connect record to the list.
     *
     */
    void AddConnectRecordToList(const std::shared_ptr<ConnectionRecord> &connRecord);

    /**
     * get the list of connect record.
     *
     */
    std::list<std::shared_ptr<ConnectionRecord>> GetConnectRecordList() const;

    /**
     * get the list of connect record.
     *
     */
    std::list<std::shared_ptr<ConnectionRecord>> GetConnectingRecordList();

    /**
     * get the count of In Progress record.
     *
     */
    uint32_t GetInProgressRecordCount();
    /**
     * remove the connect record from list.
     *
     */
    void RemoveConnectRecordFromList(const std::shared_ptr<ConnectionRecord> &connRecord);

    /**
     * check whether connect list is empty.
     *
     */
    bool IsConnectListEmpty();

    /**
     * add caller record
     *
     */
    void AddCallerRecord(const sptr<IRemoteObject> &callerToken, int requestCode, std::string srcAbilityId = "",
        uint32_t callingTokenId = 0);

    /**
     * get caller record to list.
     *
     */
    std::list<std::shared_ptr<CallerRecord>> GetCallerRecordList() const;
    std::shared_ptr<AbilityRecord> GetCallerRecord() const;

    std::shared_ptr<CallerAbilityInfo> GetCallerInfo() const;

    /**
     * get connecting record from list.
     *
     */
    std::shared_ptr<ConnectionRecord> GetConnectingRecord() const;

    /**
     * get disconnecting record from list.
     *
     */
    std::shared_ptr<ConnectionRecord> GetDisconnectingRecord() const;

    /**
     * convert ability state (enum type to string type).
     *
     */
    static std::string ConvertAbilityState(const AbilityState &state);

    static std::string ConvertAppState(const AppState &state);

    /**
     * convert life cycle state to ability state .
     *
     */
    static int ConvertLifeCycleToAbilityState(const AbilityLifeCycleState &state);

    /**
     * get the ability record id.
     *
     */
    inline int GetRecordId() const
    {
        return recordId_;
    }

    /**
     * dump ability info.
     *
     */
    void Dump(std::vector<std::string> &info);

    void DumpClientInfo(std::vector<std::string> &info, const std::vector<std::string> &params,
        bool isClient = false, bool dumpConfig = true) const;

    /**
     * Called when client complete dump.
     *
     * @param infos The dump info.
     */
    void DumpAbilityInfoDone(std::vector<std::string> &infos);

    /**
     * dump ability state info.
     *
     */
    void DumpAbilityState(std::vector<std::string> &info, bool isClient, const std::vector<std::string> &params);

    void SetStartTime();

    int64_t GetStartTime() const;

    /**
     * dump service info.
     *
     */
    void DumpService(std::vector<std::string> &info, bool isClient = false) const;

    /**
     * dump service info.
     *
     */
    void DumpService(std::vector<std::string> &info, std::vector<std::string> &params, bool isClient = false) const;

    /**
     * set aconnect remote object.
     *
     */
    void SetConnRemoteObject(const sptr<IRemoteObject> &remoteObject);

    /**
     * get connect remote object.
     *
     */
    sptr<IRemoteObject> GetConnRemoteObject() const;

    void AddStartId();
    int GetStartId() const;

    void SetIsUninstallAbility();
    /**
     * Determine whether ability is uninstalled
     *
     * @return true: uninstalled false: installed
     */
    bool IsUninstallAbility() const;
    void ShareData(const int32_t &uniqueId);
    void SetLauncherRoot();
    bool IsLauncherRoot() const;
    bool IsAbilityState(const AbilityState &state) const;
    bool IsActiveState() const;

    void SetStartSetting(const std::shared_ptr<AbilityStartSetting> &setting);
    std::shared_ptr<AbilityStartSetting> GetStartSetting() const;

    void SetRestarting(const bool isRestart);
    void SetRestarting(const bool isRestart, int32_t canReStartCount);
    int32_t GetRestartCount() const;
    void SetRestartCount(int32_t restartCount);
    bool GetKeepAlive() const;
    void SetLoading(bool status);
    bool IsLoading() const;
    int64_t GetRestartTime();
    void SetRestartTime(const int64_t restartTime);
    void SetAppIndex(const int32_t appIndex);
    int32_t GetAppIndex() const;
    bool IsRestarting() const;
    void SetAppState(const AppState &state);
    AppState GetAppState() const;

    void SetLaunchReason(const LaunchReason &reason);
    void SetLastExitReason(const ExitReason &exitReason);
    void ContinueAbility(const std::string &deviceId, uint32_t versionCode);
    void NotifyContinuationResult(int32_t result);

    void SetMissionId(int32_t missionId);
    int32_t GetMissionId() const;

    void SetUid(int32_t uid);
    int32_t GetUid();
    int32_t GetPid();
    void SetSwitchingPause(bool state);
    bool IsSwitchingPause();
    void SetOwnerMissionUserId(int32_t userId);
    int32_t GetOwnerMissionUserId();

    // new version
    ResolveResultType Resolve(const AbilityRequest &abilityRequest);
    bool ReleaseCall(const sptr<IAbilityConnection> &connect);
    bool IsNeedToCallRequest() const;
    bool IsStartedByCall() const;
    void SetStartedByCall(const bool isFlag);
    void CallRequest();
    bool CallRequestDone(const sptr<IRemoteObject> &callStub) const;
    bool IsStartToBackground() const;
    void SetStartToBackground(const bool flag);
    bool IsStartToForeground() const;
    void SetStartToForeground(const bool flag);
    void SetSessionInfo(sptr<SessionInfo> sessionInfo);
    void UpdateSessionInfo(sptr<IRemoteObject> sessionToken);
    void SetMinimizeReason(bool fromUser);
    bool IsMinimizeFromUser() const;
    void SetClearMissionFlag(bool clearMissionFlag);
    bool IsClearMissionFlag();

    void SetSpecifiedFlag(const std::string &flag);
    std::string GetSpecifiedFlag() const;
    void SetWindowMode(int32_t windowMode);
    void RemoveWindowMode();
    LifeCycleStateInfo lifeCycleStateInfo_;                // target life state info
    #ifdef ABILITY_COMMAND_FOR_TEST
    int BlockAbility();
    #endif

    bool CanRestartRootLauncher();

    bool CanRestartResident();

    std::string GetLabel();
    inline int64_t GetAbilityRecordId() const
    {
        return recordId_;
    }

    void SetPendingState(AbilityState state);
    AbilityState GetPendingState() const;

    bool IsNeedBackToOtherMissionStack();
    void SetNeedBackToOtherMissionStack(bool isNeedBackToOtherMissionStack);
    std::shared_ptr<AbilityRecord> GetOtherMissionStackAbilityRecord() const;
    void SetOtherMissionStackAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void RevokeUriPermission();
    void RemoveAbilityDeathRecipient() const;
    bool IsExistConnection(const sptr<IAbilityConnection> &connect);

    int32_t GetCollaboratorType() const;

    std::string GetMissionAffinity() const;

    void SetLockedState(bool lockedState);
    bool GetLockedState();

    void SetAttachDebug(const bool isAttachDebug);
    void SetAssertDebug(bool isAssertDebug);
    int32_t CreateModalUIExtension(const Want &want);

    AppExecFwk::ElementName GetElementName() const;
    bool IsDebugApp() const;
    bool IsDebug() const;

    void AddAbilityWindowStateMap(uint64_t uiExtensionComponentId,
        AbilityWindowState abilityWindowState);

    void RemoveAbilityWindowStateMap(uint64_t uiExtensionComponentId);

    bool IsAbilityWindowReady();

    void SetAbilityWindowState(const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd, bool isFinished);

    void SetUIExtensionAbilityId(const int32_t uiExtensionAbilityId);
    int32_t GetUIExtensionAbilityId() const;

    void OnProcessDied();

    void SetProcessName(const std::string &process);

    std::string GetProcessName() const;

    void SetURI(const std::string &uri);
    std::string GetURI() const;

    void DoBackgroundAbilityWindowDelayed(bool needBackground);
    bool BackgroundAbilityWindowDelayed();

    bool IsSceneBoard() const;

    void SetRestartAppFlag(bool isRestartApp);
    bool GetRestartAppFlag() const;

    void UpdateUIExtensionInfo(const WantParams &wantParams);

    void SetSpecifyTokenId(const uint32_t specifyTokenId);

    void SaveConnectWant(const Want &want);

    void UpdateConnectWant();

    void RemoveConnectWant();

protected:
    void SendEvent(uint32_t msg, uint32_t timeOut, int32_t param = -1);

    sptr<Token> token_ = {};                               // used to interact with kit and wms
    std::unique_ptr<LifecycleDeal> lifecycleDeal_ = {};    // life manager used to schedule life
    AbilityState currentState_ = AbilityState::INITIAL;    // current life state
    Want want_ = {};                                       // want to start this ability

private:
    /**
     * get the type of ability.
     *
     */
    void GetAbilityTypeString(std::string &typeStr);
    void OnSchedulerDied(const wptr<IRemoteObject> &remote);
    void GrantUriPermission(Want &want, std::string targetBundleName, bool isSandboxApp, uint32_t tokenId);
    void GrantDmsUriPermission(Want &want, std::string targetBundleName);
    bool IsDmsCall(Want &want);
    int32_t GetCurrentAccountId() const;

    /**
     * add system ability caller record
     *
     */
    void AddSystemAbilityCallerRecord(const sptr<IRemoteObject> &callerToken, int requestCode,
        std::string srcAbilityId);

    bool IsSystemAbilityCall(const sptr<IRemoteObject> &callerToken, uint32_t callingTokenId = 0);

#ifdef WITH_DLP
    void HandleDlpAttached();
    void HandleDlpClosed();
#endif // WITH_DLP
    void NotifyRemoveShellProcess(int32_t type);
    void NotifyAnimationAbilityDied();
    inline void SetCallerAccessTokenId(uint32_t callerAccessTokenId)
    {
        callerAccessTokenId_ = callerAccessTokenId;
    }

    bool GrantPermissionToShell(const std::vector<std::string> &uriVec, uint32_t flag, std::string targetPkg);

    void GrantUriPermissionInner(Want &want, std::vector<std::string> &uriVec, const std::string &targetBundleName,
         uint32_t tokenId);
    void GrantUriPermissionFor2In1Inner(
        Want &want, std::vector<std::string> &uriVec, const std::string &targetBundleName, uint32_t tokenId);

    LastExitReason CovertAppExitReasonToLastReason(const Reason exitReason);

    void NotifyMissionBindPid();

    void DumpUIExtensionRootHostInfo(std::vector<std::string> &info) const;

    void DumpUIExtensionPid(std::vector<std::string> &info, bool isUIExtension) const;

    bool GetUriListFromWant(Want &want, std::vector<std::string> &uriVec);

    void PublishFileOpenEvent(const Want &want);

    void SetDebugAppByWaitingDebugFlag();

#ifdef SUPPORT_SCREEN
    std::shared_ptr<Want> GetWantFromMission() const;
    void SetShowWhenLocked(const AppExecFwk::AbilityInfo &abilityInfo, sptr<AbilityTransitionInfo> &info) const;
    void SetAbilityTransitionInfo(const AppExecFwk::AbilityInfo &abilityInfo,
        sptr<AbilityTransitionInfo> &info) const;
    void SetAbilityTransitionInfo(sptr<AbilityTransitionInfo>& info) const;
    sptr<IWindowManagerServiceHandler> GetWMSHandler() const;
    void SetWindowModeAndDisplayId(sptr<AbilityTransitionInfo> &info, const std::shared_ptr<Want> &want) const;
    sptr<AbilityTransitionInfo> CreateAbilityTransitionInfo();
    sptr<AbilityTransitionInfo> CreateAbilityTransitionInfo(const std::shared_ptr<StartOptions> &startOptions,
        const std::shared_ptr<Want> &want) const;
    sptr<AbilityTransitionInfo> CreateAbilityTransitionInfo(const AbilityRequest &abilityRequest) const;
    sptr<AbilityTransitionInfo> CreateAbilityTransitionInfo(const std::shared_ptr<StartOptions> &startOptions,
        const std::shared_ptr<Want> &want, const AbilityRequest &abilityRequest);
    std::shared_ptr<Global::Resource::ResourceManager> CreateResourceManager() const;
    std::shared_ptr<Media::PixelMap> GetPixelMap(const uint32_t windowIconId,
        std::shared_ptr<Global::Resource::ResourceManager> resourceMgr) const;

    void AnimationTask(bool isRecent, const AbilityRequest &abilityRequest,
        const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<AbilityRecord> &callerAbility);
    void NotifyAnimationFromStartingAbility(const std::shared_ptr<AbilityRecord> &callerAbility,
        const AbilityRequest &abilityRequest) const;
    void NotifyAnimationFromRecentTask(const std::shared_ptr<StartOptions> &startOptions,
        const std::shared_ptr<Want> &want) const;
    void NotifyAnimationFromTerminatingAbility(const std::shared_ptr<AbilityRecord> &callerAbility, bool needExit,
        bool flag);

    void StartingWindowTask(bool isRecent, bool isCold, const AbilityRequest &abilityRequest,
        std::shared_ptr<StartOptions> &startOptions);
    void StartingWindowColdTask(bool isRecnet, const AbilityRequest &abilityRequest,
        std::shared_ptr<StartOptions> &startOptions);
    void PostCancelStartingWindowColdTask();
    void StartingWindowHot(const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want,
        const AbilityRequest &abilityRequest);
    void StartingWindowHot();
    void StartingWindowCold(const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want,
        const AbilityRequest &abilityRequest);
    void InitColdStartingWindowResource(const std::shared_ptr<Global::Resource::ResourceManager> &resourceMgr);
    void GetColdStartingWindowResource(std::shared_ptr<Media::PixelMap> &bg, uint32_t &bgColor);
    void SetAbilityStateInner(AbilityState state);
#endif

    static int64_t abilityRecordId;
    int recordId_ = 0;                                // record id
    int32_t uiExtensionAbilityId_ = 0;                // uiextension ability id
    AppExecFwk::AbilityInfo abilityInfo_ = {};             // the ability info get from BMS
    AppExecFwk::ApplicationInfo applicationInfo_ = {};     // the ability info get from BMS
    std::weak_ptr<AbilityRecord> preAbilityRecord_ = {};   // who starts this ability record
    std::weak_ptr<AbilityRecord> nextAbilityRecord_ = {};  // ability that started by this ability
    int64_t startTime_ = 0;                           // records first time of ability start
    int64_t restartTime_ = 0;                         // the time of last trying restart
    bool isReady_ = false;                            // is ability thread attached?
    bool isWindowStarted_ = false;                     // is window hotstart or coldstart?
    bool isWindowAttached_ = false;                   // Is window of this ability attached?
    bool isLauncherAbility_ = false;                  // is launcher?

    sptr<IAbilityScheduler> scheduler_ = {};       // kit scheduler
    bool isLoading_ = false;        // is loading?
    bool isTerminating_ = false;              // is terminating ?
    bool isCreateByConnect_ = false;          // is created by connect ability mode?

    int requestCode_ = -1;  // requestCode_: >= 0 for-result start mode; <0 for normal start mode in default.
    sptr<IRemoteObject::DeathRecipient> schedulerDeathRecipient_ = {};  // scheduler binderDied Recipient

    /**
     * result_: ability starts with for-result mode will send result before being terminated.
     * Its caller will receive results before active.
     * Now we assume only one result generate when terminate.
     */
    std::shared_ptr<AbilityResult> result_ = {};

    /**
     * When this ability startAbilityForResult another ability, if another ability is terminated,
     * this ability will move to foreground, during this time, isAbilityForegrounding_ is true,
     * isAbilityForegrounding_ will be set to false when this ability is background
     */
    bool isAbilityForegrounding_ = false;

    // service(ability) can be connected by multi-pages(abilites), so need to store this service's connections
    mutable ffrt::mutex connRecordListMutex_;
    std::list<std::shared_ptr<ConnectionRecord>> connRecordList_ = {};
    // service(ability) onConnect() return proxy of service ability
    sptr<IRemoteObject> connRemoteObject_ = {};
    int startId_ = 0;  // service(ability) start id

    // page(ability) can be started by multi-pages(abilites), so need to store this ability's caller
    std::list<std::shared_ptr<CallerRecord>> callerList_ = {};

    bool isUninstall_ = false;

    bool isLauncherRoot_ = false;

    PacMap stateDatas_;             // ability saved ability state data
    WindowConfig windowConfig_;
    bool isRestarting_ = false;     // is restarting ?
    AppState appState_ = AppState::BEGIN;

    int32_t uid_ = 0;
    int32_t pid_ = 0;
    int32_t missionId_ = -1;
    int32_t ownerMissionUserId_ = -1;
    bool isSwitchingPause_ = false;

    // new version
    std::shared_ptr<CallContainer> callContainer_ = nullptr;
    bool isStartedByCall_ = false;
    bool isStartToBackground_ = false;
    bool isStartToForeground_ = false;
    int32_t appIndex_ = 0;
    bool minimizeReason_ = false;

    bool clearMissionFlag_ = false;

    int32_t restartCount_ = -1;
    int32_t restartMax_ = -1;
    std::string specifiedFlag_;
    std::string uri_;
    ffrt::mutex lock_;
    mutable ffrt::mutex dumpInfoLock_;
    mutable ffrt::mutex dumpLock_;
    mutable ffrt::mutex resultLock_;
    mutable ffrt::mutex wantLock_;
    mutable ffrt::condition_variable dumpCondition_;
    mutable bool isDumpTimeout_ = false;
    std::vector<std::string> dumpInfos_;
    std::atomic<AbilityState> pendingState_ = AbilityState::INITIAL;    // pending life state
    std::atomic<AbilityVisibilityState> abilityVisibilityState_ = AbilityVisibilityState::INITIAL;

    // scene session
    sptr<SessionInfo> sessionInfo_ = nullptr;
    mutable ffrt::mutex sessionLock_;
    std::map<uint64_t, AbilityWindowState> abilityWindowStateMap_;

#ifdef SUPPORT_SCREEN
    bool isStartingWindow_ = false;
    uint32_t bgColor_ = 0;
    std::shared_ptr<Media::PixelMap> startingWindowBg_ = nullptr;

    bool isCompleteFirstFrameDrawing_ = false;
    bool coldStart_ = false;
#endif

    bool isGrantedUriPermission_ = false;
    uint32_t callerAccessTokenId_ = -1;
    bool isNeedBackToOtherMissionStack_ = false;
    std::weak_ptr<AbilityRecord> otherMissionStackAbilityRecord_; // who starts this ability record by SA
    int32_t collaboratorType_ = 0;
    std::string missionAffinity_ = "";
    bool lockedState_ = false;
    bool isAttachDebug_ = false;
    bool isAssertDebug_ = false;
    bool isAppAutoStartup_ = false;
    bool isConnected = false;
    std::atomic_bool backgroundAbilityWindowDelayed_ = false;

    bool isRestartApp_ = false; // Only app calling RestartApp can be set to true
    uint32_t specifyTokenId_ = 0;

    std::shared_ptr<Want> connectWant_ = nullptr;
    ffrt::mutex connectWantLock_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
