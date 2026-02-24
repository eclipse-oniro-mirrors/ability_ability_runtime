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

#include "ability_record.h"

#include <singleton.h>

#include "ability_manager_service.h"
#include "ability_resident_process_rdb.h"
#include "ability_scheduler_stub.h"
#include "agent_extension_connection_constants.h"
#include "app_exit_reason_data_manager.h"
#include "appfreeze_manager.h"
#include "app_utils.h"
#include "array_wrapper.h"
#include "accesstoken_kit.h"
#include "configuration_convertor.h"
#include "connection_state_manager.h"
#include "common_event_manager.h"
#include "error_msg_util.h"
#include "foreground_app_connection_manager.h"
#include "freeze_util.h"
#include "global_constant.h"
#include "hitrace_meter.h"
#include "image_source.h"
#include "json_utils.h"
#include "last_exit_detail_info.h"
#include "main_element_utils.h"
#include "multi_instance_utils.h"
#include "os_account_manager_wrapper.h"
#include "ui_service_extension_connection_constants.h"
#include "res_sched_util.h"
#include "scene_board_judgement.h"
#include "startup_util.h"
#include "system_ability_token_callback.h"
#include "ui_extension_wrapper.h"
#ifdef SUPPORT_UPMS
#include "uri_permission_manager_client.h"
#endif // SUPPORT_UPMS
#include "param.h"
#include "permission_constants.h"
#include "process_options.h"
#include "uri_utils.h"
#include "user_controller/user_controller.h"
#include "utils/state_utils.h"
#ifdef SUPPORT_GRAPHICS
#include "image_source.h"
#endif
#ifdef SUPPORT_SCREEN
#include "locale_config.h"
#endif

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AAFwk {
using namespace OHOS::Security;
using namespace OHOS::AAFwk::PermissionConstants;
using namespace OHOS::AbilityRuntime::GlobalConstant;
const std::string ERROR_INFO_ENHANCE = "errorInfoEnhance";
const std::string MULTI_THREAD = "multiThread";
const std::string DMS_PROCESS_NAME = "distributedsched";
const std::string DMS_MISSION_ID = "dmsMissionId";
const std::string DMS_SRC_NETWORK_ID = "dmsSrcNetworkId";
const std::string ABILITY_OWNER_USERID = "AbilityMS_Owner_UserId";
const std::u16string SYSTEM_ABILITY_TOKEN_CALLBACK = u"ohos.aafwk.ISystemAbilityTokenCallback";
const std::string SHOW_ON_LOCK_SCREEN = "ShowOnLockScreen";
#ifdef WITH_DLP
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
#endif // WITH_DLP
const std::string COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
const std::string KEY_MISSION_ID = "ohos.anco.param.missionId";
const std::string PARAMS_FILE_SAVING_URL_KEY = "pick_path_return";
const std::string SHELL_ASSISTANT_DIEREASON = "crash_die";
const std::string PARAM_MISSION_AFFINITY_KEY = "ohos.anco.param.missionAffinity";
const std::string DISTRIBUTED_FILES_PATH = "/data/storage/el2/distributedfiles/";
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
const std::string UIEXTENSION_ROOT_HOST_PID = "ability.want.params.uiExtensionRootHostPid";
const std::string UIEXTENSION_HOST_PID = "ability.want.params.uiExtensionHostPid";
const std::string UIEXTENSION_HOST_UID = "ability.want.params.uiExtensionHostUid";
const std::string UIEXTENSION_HOST_BUNDLENAME = "ability.want.params.uiExtensionHostBundleName";
const std::string UIEXTENSION_BIND_ABILITY_ID = "ability.want.params.uiExtensionBindAbilityId";
const std::string UIEXTENSION_NOTIFY_BIND = "ohos.uiextension.params.notifyProcessBind";
constexpr const char* PARAM_SEND_RESULT_CALLER_BUNDLENAME = "ohos.anco.param.sendResultCallderBundleName";
constexpr const char* PARAM_SEND_RESULT_CALLER_TOKENID = "ohos.anco.param.sendResultCallerTokenId";
constexpr const char* PARAM_RESV_ANCO_IS_NEED_UPDATE_NAME = "ohos.anco.param.isNeedUpdateName";
constexpr const char* DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
constexpr const char* SPECIFIED_ABILITY_FLAG = "ohos.ability.params.specifiedAbilityFlag";
constexpr const char* UIEXTENSION_LAUNCH_TIMESTAMP_HIGH = "ohos.ability.params.uiExtensionLaunchTimestampHigh";
constexpr const char* UIEXTENSION_LAUNCH_TIMESTAMP_LOW = "ohos.ability.params.uiExtensionLaunchTimestampLow";
// Developer mode param
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
constexpr const char* APP_PROVISION_TYPE_DEBUG = "debug";
constexpr const char* DMS_CALLER_BUNDLE_NAME = "ohos.dms.param.sourceCallerBundleName";
constexpr const char* DMS_CALLER_ABILITY_NAME = "ohos.dms.param.sourceCallerAbilityName";
constexpr const char* DMS_CALLER_NATIVE_NAME = "ohos.dms.param.sourceCallerNativeName";
constexpr const char* DMS_CALLER_APP_ID = "ohos.dms.param.sourceCallerAppId";
constexpr const char* DMS_CALLER_APP_IDENTIFIER = "ohos.dms.param.sourceCallerAppIdentifier";
constexpr const char* IS_HOOK = "ohos.ability_runtime.is_hook";
const int32_t SHELL_ASSISTANT_DIETYPE = 0;
std::atomic<int64_t> AbilityRecord::abilityRecordId = 0;
const int32_t DEFAULT_USER_ID = 0;
const int32_t SEND_RESULT_CANCELED = -1;
const int32_t DEFAULT_REQUEST_CODE = -1;
const int VECTOR_SIZE = 2;
const int LOAD_TIMEOUT_ASANENABLED = 150;
const int TERMINATE_TIMEOUT_ASANENABLED = 150;
const int HALF_TIMEOUT = 2;
const int MAX_URI_COUNT = 500;
const int RESTART_SCENEBOARD_DELAY = 500;
constexpr int32_t DMS_UID = 5522;
constexpr int32_t SCHEDULER_DIED_TIMEOUT = 60000;
const std::string JSON_KEY_ERR_MSG = "errMsg";
const int32_t BY_CALL_HALF_TIMEOUT_MS = 2500;
const int32_t BY_CALL_TIMEOUT_MS = 5000;

auto g_addLifecycleEventTask = [](sptr<Token> token, std::string &methodName) {
    CHECK_POINTER_LOG(token, "token is nullptr");
    std::string entry = std::string("AbilityRecord::") + methodName + "; the " + methodName + " lifecycle starts.";
    FreezeUtil::GetInstance().AddLifecycleEvent(token->AsObject(), entry);
};

AbilityRecord::AbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode)
    : want_(want), requestCode_(requestCode), abilityInfo_(abilityInfo)
{
    abilityInfo_.applicationInfo = applicationInfo;
    recordId_ = abilityRecordId++;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (abilityMgr) {
        bool isRootLauncher = (abilityInfo_.applicationInfo.bundleName == AbilityConfig::LAUNCHER_BUNDLE_NAME);
        restartMax_ = AmsConfigurationParameter::GetInstance().GetMaxRestartNum(isRootLauncher);
        bool flag = abilityMgr->GetStartUpNewRuleFlag();
        want_.SetParam(COMPONENT_STARTUP_NEW_RULES, flag);
    }
    restartCount_ = restartMax_;
    isAppAutoStartup_ = want_.GetBoolParam(Want::PARAM_APP_AUTO_STARTUP_LAUNCH_REASON, false);
    if (want_.HasParameter(Want::PARAM_APP_AUTO_STARTUP_LAUNCH_REASON)) {
        want_.RemoveParam(Want::PARAM_APP_AUTO_STARTUP_LAUNCH_REASON);
    }
    SetDebugAppByWaitingDebugFlag();
    launchDebugInfo_.Update(want_);
}

AbilityRecord::~AbilityRecord()
{
    if (token_) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(token_->AsObject());
    }
    FreezeUtil::GetInstance().DeleteAppLifecycleEvent(GetPid());
    if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto object = scheduler_->AsObject();
        if (object != nullptr) {
            object->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
    want_.CloseAllFd();
}

std::shared_ptr<AbilityRecord> AbilityRecord::CreateAbilityRecord(const AbilityRequest &abilityRequest)
{
    auto abilityRecord = std::make_shared<AbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    
    abilityRecord->Init(abilityRequest);
    return abilityRecord;
}

void AbilityRecord::Init(const AbilityRequest &abilityRequest)
{
    SetUid(abilityRequest.uid);
    int32_t appIndex = 0;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(abilityRequest.want, appIndex);
    SetAppIndex(appIndex);
    SetSecurityFlag(abilityRequest.want.GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false));
    SetCallerAccessTokenId(abilityRequest.callerAccessTokenId);
    if (abilityRequest.processOptions != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "isPreloadStart:%{public}d", abilityRequest.processOptions->isPreloadStart);
        SetPreloadStart(abilityRequest.processOptions->isPreloadStart);
    }
    sessionInfo_ = abilityRequest.sessionInfo;
    if (AppUtils::GetInstance().IsMultiProcessModel() && abilityRequest.abilityInfo.isStageBasedModel &&
        abilityRequest.abilityInfo.type == AppExecFwk::AbilityType::PAGE && !abilityRequest.customProcess.empty()) {
        SetCustomProcessFlag(abilityRequest.customProcess);
    }
    if (abilityRequest.sessionInfo != nullptr) {
        instanceKey_ = abilityRequest.sessionInfo->instanceKey;
    }

    lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    token_ = sptr<Token>::MakeSptr(weak_from_this());
    if (abilityInfo_.applicationInfo.isLauncherApp) {
        isLauncherAbility_ = true;
    }

    if (abilityRequest.startSetting != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityRequest.startSetting...");
        SetStartSetting(abilityRequest.startSetting);
    }
    if (abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityRequest.callType is CALL_REQUEST_TYPE.");
        SetStartedByCall(true);
    }
    // Initialize plugin ability relationship
    InitPluginAbility(abilityRequest);
    collaboratorType_ = abilityRequest.collaboratorType;
    missionAffinity_ = abilityRequest.want.GetStringParam(PARAM_MISSION_AFFINITY_KEY);

    auto userId = abilityRequest.appInfo.uid / BASE_USER_RANGE;
    if ((userId == 0 || AppUtils::GetInstance().InResidentWhiteList(abilityRequest.abilityInfo.bundleName)) &&
        DelayedSingleton<ResidentProcessManager>::GetInstance()->IsResidentAbility(
            abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name, userId)) {
        keepAliveBundle_ = true;
    }
}

AbilityRecordType AbilityRecord::GetAbilityRecordType()
{
    return AbilityRecordType::BASE_ABILITY;
}

void AbilityRecord::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t AbilityRecord::GetUid()
{
    return uid_;
}

pid_t AbilityRecord::GetPid() const
{
    return pid_;
}

void AbilityRecord::SetPid(pid_t pid)
{
    pid_ = pid;
}

void AbilityRecord::LoadUIAbility()
{
    SetLoading(true);
    int loadTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * LOAD_TIMEOUT_MULTIPLE;
    if (abilityInfo_.applicationInfo.asanEnabled || abilityInfo_.applicationInfo.tsanEnabled) {
        loadTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * LOAD_TIMEOUT_ASANENABLED;
    } else {
        int coldStartTimeout =
            AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * COLDSTART_TIMEOUT_MULTIPLE;
        std::lock_guard guard(wantLock_);
        loadTimeout = want_.GetBoolParam("coldStart", false) ? coldStartTimeout : loadTimeout;
    }
    if (IsPreloadStart() && !IsPreloaded()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "preload, no event sent");
        return;
    }
    SendEvent(AbilityManagerService::LOAD_HALF_TIMEOUT_MSG, loadTimeout / HALF_TIMEOUT);
    SendEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, loadTimeout);
    std::string methodName = "LoadAbility";
    g_addLifecycleEventTask(token_, methodName);
}

int AbilityRecord::LoadAbility(bool isShellCall, bool isStartupHide, pid_t callingPid,
    uint64_t loadAbilityCallbackId, pid_t selfPid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadLifecycle: abilityName:%{public}s", abilityInfo_.name.c_str());
    startTime_ = AbilityUtil::SystemTimeMillis();
    lifeCycleStateInfo_.launchParam.launchUptime = startTime_;
    lifeCycleStateInfo_.launchParam.launchUTCTime = AbilityUtil::UTCTimeMillis();
    CHECK_POINTER_AND_RETURN(token_, ERR_INVALID_VALUE);
    // only for UIAbility
    if (!IsDebug() && abilityInfo_.type != AppExecFwk::AbilityType::DATA) {
        LoadUIAbility();
    }

    std::string appName = abilityInfo_.applicationInfo.name;
    CHECK_TRUE_RETURN_RET(appName.empty(), ERR_INVALID_VALUE, "app name empty");
    CHECK_TRUE_RETURN_RET(!CanRestartRootLauncher(), ERR_INVALID_VALUE, "root launcher restart out of max");

    if (isRestarting_) {
        restartTime_ = AbilityUtil::SystemTimeMillis();
    }
    sptr<Token> callerToken = nullptr;
    auto caller = GetCallerRecord();
    if (caller) {
        callerToken = caller->GetToken();
    }
    std::lock_guard guard(wantLock_);
    want_.SetParam(ABILITY_OWNER_USERID, ownerMissionUserId_);
    want_.SetParam(IS_HOOK, isHook_);
    firstCallerBundleName_ = want_.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    AbilityRuntime::LoadParam loadParam;
    loadParam.abilityRecordId = recordId_;
    loadParam.isShellCall = Rosen::SceneBoardJudgement::IsSceneBoardEnabled() ? isShellCall
        : AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    loadParam.token = token_;
    loadParam.preToken = callerToken;
    loadParam.instanceKey = instanceKey_;
    loadParam.isCallerSetProcess = IsCallerSetProcess();
    loadParam.customProcessFlag = customProcessFlag_;
    loadParam.isStartupHide = isStartupHide;
    loadParam.callingPid = callingPid;
    loadParam.loadAbilityCallbackId = loadAbilityCallbackId;
    loadParam.isPrelaunch = isPrelaunch_;
    loadParam.isPreloadStart = isPreloadStart_;
    loadParam.selfPid = selfPid;
    auto userId = abilityInfo_.uid / BASE_USER_RANGE;
    bool isMainUIAbility =
        MainElementUtils::IsMainUIAbility(abilityInfo_.bundleName, abilityInfo_.name, userId);
    MainElementUtils::SetMainUIAbilityKeepAliveFlag(isMainUIAbility,
        abilityInfo_.bundleName, loadParam);
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
        loadParam, abilityInfo_, abilityInfo_.applicationInfo, want_);
    want_.RemoveParam(IS_HOOK);
    want_.RemoveParam(ABILITY_OWNER_USERID);
    want_.RemoveParam(Want::PARAMS_REAL_CALLER_KEY);
    if (DelayedSingleton<AppScheduler>::GetInstance()->IsAttachDebug(abilityInfo_.bundleName)) {
        SetAttachDebug(true);
    }
    if (result == ERR_OK) {
        AbilityRuntime::UserController::GetInstance().AddToUserLockedBundleList(abilityInfo_.bundleName, userId);
    }
    return result;
}

bool AbilityRecord::CanRestartRootLauncher()
{
    if (isLauncherRoot_ && isRestarting_ && IsLauncherAbility() && (restartCount_ < 0)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "root launcher restart out of max");
        return false;
    }
    return true;
}

bool AbilityRecord::CanRestartResident()
{
    auto isKeepAlive = GetKeepAlive();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isKeepAlive: %{public}d, isRestarting: %{public}d, restartCount: %{public}d",
        isKeepAlive, isRestarting_, restartCount_);
    if (isKeepAlive && isRestarting_ && (restartCount_ < 0)) {
        int restartIntervalTime = 0;
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr) {
            restartIntervalTime = AmsConfigurationParameter::GetInstance().GetRestartIntervalTime();
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "restartTime: %{public}lld, now: %{public}lld, intervalTine:%{public}d",
            static_cast<unsigned long long>(restartTime_),
            static_cast<unsigned long long>(AbilityUtil::SystemTimeMillis()), restartIntervalTime);
        if ((AbilityUtil::SystemTimeMillis() - restartTime_) < restartIntervalTime) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "resident restart out of max");
            return false;
        }
    }
    return true;
}

// only for UIAbility
void AbilityRecord::ForegroundAbility(uint32_t sceneFlag, bool hasLastWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    isWindowStarted_ = true;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ForegroundLifecycle %{public}s", abilityInfo_.name.c_str());
    CHECK_POINTER(lifecycleDeal_);

    // schedule active after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions
#ifdef SUPPORT_SCREEN
    SetAbilityStateInner(AbilityState::FOREGROUNDING);
#endif // SUPPORT_SCREEN
    lifeCycleStateInfo_.sceneFlag = sceneFlag;
    Want want;
    if (hasLastWant) {
        if (HasLastWant()) {
            SetWant(*lastWant_);
            lifeCycleStateInfo_.isNewWant = true;
            lastWant_ = nullptr;
        }
        SetBackgroundDrivenFlag(false);
    }
    want = GetWant();
    UpdateDmsCallerInfo(want);
    AbilityRuntime::ErrorMsgGuard errorMsgGuard(token_ ? token_->AsObject() : nullptr,
        reinterpret_cast<uintptr_t>(GetScheduler().GetRefPtr()), "ScheduleAbilityTransaction");
    if (AAFwk::AppUtils::GetInstance().IsStartSpecifiedProcess() && GetApplicationInfo().isSystemApp) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetSpecifiedFlag: %{public}s", GetSpecifiedFlag().c_str());
        want.SetParam(SPECIFIED_ABILITY_FLAG, GetSpecifiedFlag());
    }
    lifecycleDeal_->ForegroundNew(want, lifeCycleStateInfo_, GetSessionInfo());
    SetIsNewWant(false);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        lifeCycleStateInfo_.sceneFlag = 0;
        lifeCycleStateInfo_.sceneFlagBak = 0;
    }
    {
        std::lock_guard guard(wantLock_);
        InsightIntentExecuteParam::RemoveInsightIntent(want_);
        isLaunching_ = false;
    }
}

void AbilityRecord::ForegroundUIExtensionAbility(uint32_t sceneFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ForegroundUIExtensionAbility:%{public}s/%{public}s",
        GetElementName().GetBundleName().c_str(), GetElementName().GetAbilityName().c_str());
    CHECK_POINTER(lifecycleDeal_);

    if (IsAbilityState(AbilityState::BACKGROUND)) {
        SendAppStartupTypeEvent(AppExecFwk::AppStartType::HOT);
    }
    // schedule active after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
#ifdef SUPPORT_SCREEN
    SetAbilityStateInner(AbilityState::FOREGROUNDING);
#endif // SUPPORT_SCREEN
    lifeCycleStateInfo_.sceneFlag = sceneFlag;
    lifecycleDeal_->ForegroundNew(GetWant(), lifeCycleStateInfo_, GetSessionInfo());
    lifeCycleStateInfo_.sceneFlag = 0;
    lifeCycleStateInfo_.sceneFlagBak = 0;
    {
        std::lock_guard guard(wantLock_);
        InsightIntentExecuteParam::RemoveInsightIntent(want_);
        isLaunching_ = false;
    }
}

void AbilityRecord::ProcessForegroundAbility(uint32_t tokenId, const ForegroundOptions &options)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability record: %{public}s/%{public}s", GetElementName().GetBundleName().c_str(),
        GetElementName().GetAbilityName().c_str());
#ifdef SUPPORT_UPMS
    {
        std::lock_guard guard(wantLock_);
        std::string targetBundleName = abilityInfo_.applicationInfo.bundleName;
        bool isNotifyCollaborator = !options.targetGrantBundleName.empty();
        if (isNotifyCollaborator) {
            targetBundleName = options.targetGrantBundleName;
        }
        GrantUriPermission(want_, targetBundleName, false, tokenId, isNotifyCollaborator);
    }
#endif // SUPPORT_UPMS
    if (!isReady_) {
        isPrelaunch_ = false;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "To load ability.");
        lifeCycleStateInfo_.sceneFlagBak = options.sceneFlag;
        LoadAbility(options.isShellCall, options.isStartupHide, options.callingPid,
            options.loadAbilityCallbackId, options.selfPid);
        return;
    }

    if (options.requestCode != DEFAULT_REQUEST_CODE &&
        ForegroundAppConnectionManager::IsForegroundAppConnection(GetAbilityInfo(), GetCallerRecord())) {
        ReportAbilityConnectionRelations();
    }

    PostForegroundTimeoutTask();
    if (IsAbilityState(AbilityState::FOREGROUND)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Activate %{public}s/%{public}s", GetElementName().GetBundleName().c_str(),
            GetElementName().GetAbilityName().c_str());
        if (IsFrozenByPreload()) {
            SetFrozenByPreload(false);
            auto ret =
                DelayedSingleton<AppScheduler>::GetInstance()->NotifyPreloadAbilityStateChanged(token_, false);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyPreloadAbilityStateChanged by start, ret: %{public}d", ret);
        }
        ForegroundAbility(options.sceneFlag);
        return;
    }
    // background to active state
    TAG_LOGD(AAFwkTag::ABILITYMGR, "MoveToForeground, %{public}s/%{public}s", GetElementName().GetBundleName().c_str(),
        GetElementName().GetAbilityName().c_str());
    lifeCycleStateInfo_.sceneFlagBak = options.sceneFlag;
    ResSchedUtil::GetInstance().ReportEventToRSS(GetUid(), GetAbilityInfo().bundleName,
        "THAW_BY_FOREGROUND_ABILITY", GetPid(), GetCallerRecord() ? GetCallerRecord()->GetPid() : -1);
    if (isPrelaunch_) {
        SendAppStartupTypeEvent(AppExecFwk::AppStartType::WARM, AppExecFwk::AppStartReason::PRE_LAUNCH);
        isPrelaunch_ = false;
    } else {
        SendAppStartupTypeEvent(AppExecFwk::AppStartType::HOT);
    }
    SetAbilityStateInner(AbilityState::FOREGROUNDING);
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token_);
}

void AbilityRecord::PostForegroundTimeoutTask()
{
    if (IsDebug() || (IsPreloadStart() && !IsPreloaded())) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "isDebug=%{public}d, isPreload=%{public}d, preloaded=%{public}d, no event sent",
            IsDebug(), IsPreloadStart(), IsPreloaded());
        return;
    }
    int foregroundTimeout =
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * FOREGROUND_TIMEOUT_MULTIPLE;
    if (InsightIntentExecuteParam::IsInsightIntentExecute(GetWant())) {
        foregroundTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() *
            INSIGHT_INTENT_TIMEOUT_MULTIPLE;
    }
    SendEvent(AbilityManagerService::FOREGROUND_HALF_TIMEOUT_MSG, foregroundTimeout / HALF_TIMEOUT);
    SendEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, foregroundTimeout);
    std::string methodName = "ProcessForegroundAbility";
    g_addLifecycleEventTask(token_, methodName);
    ResSchedUtil::GetInstance().ReportLoadingEventToRss(LoadingStage::FOREGROUND_BEGIN, GetPid(), GetUid(),
        foregroundTimeout, GetAbilityRecordId());
}

void AbilityRecord::RemoveForegroundTimeoutTask()
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER(handler);
    handler->RemoveEvent(AbilityManagerService::FOREGROUND_HALF_TIMEOUT_MSG, GetAbilityRecordId());
    handler->RemoveEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, GetAbilityRecordId());
}

void AbilityRecord::RemoveLoadTimeoutTask()
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER(handler);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "RemoveLoadTimeoutTask:%{public}" PRId64, GetAbilityRecordId());
    handler->RemoveEvent(AbilityManagerService::LOAD_HALF_TIMEOUT_MSG, GetAbilityRecordId());
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, GetAbilityRecordId());
}

#ifdef SUPPORT_SCREEN
void AbilityRecord::SetAbilityTransitionInfo(sptr<AbilityTransitionInfo>& info) const
{
    info->abilityToken_ = token_;
    info->missionId_ = missionId_;
    info->abilityName_ = abilityInfo_.name;
    info->bundleName_ = abilityInfo_.bundleName;
    info->windowModes_ = abilityInfo_.windowModes;
    info->maxWindowRatio_ = abilityInfo_.maxWindowRatio;
    info->minWindowRatio_ = abilityInfo_.minWindowRatio;
    info->maxWindowWidth_ = abilityInfo_.maxWindowWidth;
    info->minWindowWidth_ = abilityInfo_.minWindowWidth;
    info->maxWindowHeight_ = abilityInfo_.maxWindowHeight;
    info->minWindowHeight_ = abilityInfo_.minWindowHeight;
    info->orientation_ = abilityInfo_.orientation;
    info->apiCompatibleVersion_ = abilityInfo_.applicationInfo.apiCompatibleVersion;
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo()
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    SetAbilityTransitionInfo(info);
    SetStartingWindow(true);
    return info;
}

std::shared_ptr<Want> AbilityRecord::GetWantFromMission() const
{
    auto missionListWrap = DelayedSingleton<AbilityManagerService>::GetInstance()->GetMissionListWrap();
    if (missionListWrap == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null missionListWrap");
        return nullptr;
    }

    InnerMissionInfo innerMissionInfo;
    int getMission = missionListWrap->GetInnerMissionInfoById(missionId_, innerMissionInfo);
    if (getMission != ERR_OK) {
        TAG_LOGE(
            AAFwkTag::ABILITYMGR, "no find mission info by missionId: %{public}d", missionId_);
        return nullptr;
    }

    return std::make_shared<Want>(innerMissionInfo.missionInfo.want);
}

void AbilityRecord::SetShowWhenLocked(const AppExecFwk::AbilityInfo &abilityInfo,
    sptr<AbilityTransitionInfo> &info) const
{
    for (const auto &data : abilityInfo.metaData.customizeData) {
        if (data.name == SHOW_ON_LOCK_SCREEN) {
            info->isShowWhenLocked_ = true;
            break;
        }
    }
}

void AbilityRecord::SetAbilityTransitionInfo(const AppExecFwk::AbilityInfo &abilityInfo,
    sptr<AbilityTransitionInfo> &info) const
{
    info->abilityName_ = abilityInfo.name;
    info->bundleName_ = abilityInfo.bundleName;
    info->windowModes_ = abilityInfo.windowModes;
    info->orientation_ = abilityInfo.orientation;
    info->apiCompatibleVersion_ = abilityInfo.applicationInfo.apiCompatibleVersion;
    SetShowWhenLocked(abilityInfo, info);
}

sptr<IWindowManagerServiceHandler> AbilityRecord::GetWMSHandler() const
{
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMgr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, fail", __func__);
        return nullptr;
    }
    return abilityMgr->GetWMSHandler();
}

void AbilityRecord::SetWindowModeAndDisplayId(sptr<AbilityTransitionInfo> &info,
    const std::shared_ptr<Want> &want) const
{
    if (!want) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, want invalid", __func__);
        return;
    }
    auto mode = want->GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    auto displayId = want->GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -1);
    if (mode != -1) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s: origin window mode: %{public}d", __func__, mode);
        info->mode_ = static_cast<uint32_t>(mode);
    }
    if (displayId != -1) {
        info->displayId_ = static_cast<uint64_t>(displayId);
    }
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want) const
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    if (startOptions != nullptr) {
        info->mode_ = static_cast<uint32_t>(startOptions->GetWindowMode());
        TAG_LOGI(AAFwkTag::ABILITYMGR, "window mode:%{public}d", info->mode_);
        info->displayId_ = static_cast<uint64_t>(startOptions->GetDisplayID());
    } else {
        SetWindowModeAndDisplayId(info, want);
    }
    return info;
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(const AbilityRequest &abilityRequest) const
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    auto abilityStartSetting = abilityRequest.startSetting;
    if (abilityStartSetting) {
        auto windowMode = abilityStartSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY);
        auto displayId = abilityStartSetting->GetProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
        try {
            info->mode_ = static_cast<uint32_t>(std::stoi(windowMode));
            info->displayId_ = static_cast<uint64_t>(std::stoi(displayId));
        } catch (...) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "windowMode: stoi(%{public}s) failed", windowMode.c_str());
            TAG_LOGW(AAFwkTag::ABILITYMGR, "displayId: stoi(%{public}s) failed", displayId.c_str());
        }
    } else {
        SetWindowModeAndDisplayId(info, std::make_shared<Want>(abilityRequest.want));
    }
    return info;
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want,
    const AbilityRequest &abilityRequest)
{
    sptr<AbilityTransitionInfo> info;
    if (startOptions) {
        info = CreateAbilityTransitionInfo(startOptions, want);
    } else {
        info = CreateAbilityTransitionInfo(abilityRequest);
    }

    SetAbilityTransitionInfo(info);
    SetStartingWindow(true);
    return info;
}

bool AbilityRecord::ReportAtomicServiceDrawnCompleteEvent()
{
    if (abilityInfo_.applicationInfo.bundleType != AppExecFwk::BundleType::ATOMIC_SERVICE) {
        return false;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Report atomic service first frame complete event.");
    AAFwk::EventInfo eventInfo;
    eventInfo.abilityName = abilityInfo_.name;
    eventInfo.moduleName = abilityInfo_.moduleName;
    eventInfo.bundleName = abilityInfo_.bundleName;
    auto eventName = AAFwk::EventName::ATOMIC_SERVICE_DRAWN_COMPLETE;
    AAFwk::EventReport::SendAtomicServiceEvent(eventName, HISYSEVENT_BEHAVIOR, eventInfo);
    return true;
}

void AbilityRecord::SetCompleteFirstFrameDrawing(const bool flag)
{
    isCompleteFirstFrameDrawing_ = flag;
}

bool AbilityRecord::IsCompleteFirstFrameDrawing() const
{
    return isCompleteFirstFrameDrawing_;
}

bool AbilityRecord::GetColdStartFlag()
{
    return coldStart_;
}

void AbilityRecord::SetColdStartFlag(bool isColdStart)
{
    coldStart_ = isColdStart;
}
#endif

void AbilityRecord::BackgroundAbility(const Closure &task)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "BackgroundLifecycle %{public}s/%{public}s",
        GetElementName().GetBundleName().c_str(), GetElementName().GetAbilityName().c_str());
    if (lifecycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null lifecycleDeal_");
        return;
    }

    if (!IsDebug()) {
        auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
        if (handler && task) {
            int backgroundTimeout =
                AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * BACKGROUND_TIMEOUT_MULTIPLE;
            if (InsightIntentExecuteParam::IsInsightIntentExecute(GetWant())) {
                backgroundTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() *
                    INSIGHT_INTENT_TIMEOUT_MULTIPLE;
            }
            handler->SubmitTask(task, "background_" + std::to_string(recordId_), backgroundTimeout, false);

            if (abilityInfo_.type == AppExecFwk::AbilityType::PAGE) {
                std::string methodName = "BackgroundAbility";
                g_addLifecycleEventTask(token_, methodName);
            }
        }
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "debug mode, no need to handle");
    }

    if (!IsTerminating() || IsRestarting()) {
        // schedule save ability state before moving to background.
        SaveAbilityState();
    }

    // schedule background after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
#ifdef SUPPORT_SCREEN
    SetAbilityStateInner(AbilityState::BACKGROUNDING);
#endif // SUPPORT_SCREEN
    lifecycleDeal_->BackgroundNew(GetWant(), lifeCycleStateInfo_, GetSessionInfo());
    std::lock_guard guard(wantLock_);
    isLaunching_ = false;
}

bool AbilityRecord::PrepareTerminateAbility(bool isSCBCall)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (lifecycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null lifecycleDeal_");
        return false;
    }
    if (!isSCBCall) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "triggered by user clicking window x");
        return lifecycleDeal_->PrepareTerminateAbility();
    }
    // execute onPrepareToTerminate until timeout
    std::unique_lock<std::mutex> lock(isPrepareTerminateAbilityMutex_);
    isPrepareTerminateAbilityCalled_.store(true);
    isPrepareTerminate_ = false;
    isPrepareTerminateAbilityDone_.store(false);
    auto condition = [weak = weak_from_this()] {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null self");
            return false;
        }
        return self->isPrepareTerminateAbilityDone_.load();
    };
    auto task = [weak = weak_from_this()]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord, prepareTerminateAbility error");
            return;
        }
        if (!self->lifecycleDeal_ || !self->lifecycleDeal_->PrepareTerminateAbility()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "PrepareTerminateAbility error");
            self->isPrepareTerminateAbilityDone_.store(true);
            self->isPrepareTerminateAbilityCalled_.store(false);
            self->isPrepareTerminateAbilityCv_.notify_all();
        }
    };
    ffrt::submit(task);
    if (!isPrepareTerminateAbilityCv_.wait_for(lock,
        std::chrono::milliseconds(GlobalConstant::PREPARE_TERMINATE_TIMEOUT_TIME), condition)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "wait timeout");
        return false;
    }
    return isPrepareTerminate_;
}

void AbilityRecord::PrepareTerminateAbilityDone(bool isTerminate)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call PrepareTerminateAbilityDone");
    std::unique_lock<std::mutex> lock(isPrepareTerminateAbilityMutex_);
    if (!isPrepareTerminateAbilityCalled_.load()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "prepare terminate not called");
        return;
    }
    isPrepareTerminate_ = isTerminate;
    isPrepareTerminateAbilityDone_.store(true);
    isPrepareTerminateAbilityCalled_.store(false);
    isPrepareTerminateAbilityCv_.notify_one();
}

void AbilityRecord::CancelPrepareTerminate()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "canceling prepare terminate,bundle=%{public}s,module=%{public}s,ability=%{public}s",
        abilityInfo_.bundleName.c_str(), abilityInfo_.moduleName.c_str(), abilityInfo_.name.c_str());
    std::unique_lock<std::mutex> lock(isPrepareTerminateAbilityMutex_);
    if (!isPrepareTerminateAbilityCalled_.load()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "prepare terminate not called");
        return;
    }
    isPrepareTerminate_ = true; // no need to terminate again
    isPrepareTerminateAbilityDone_.store(true);
    isPrepareTerminateAbilityCalled_.store(false);
    isPrepareTerminateAbilityCv_.notify_one();
}

int AbilityRecord::TerminateAbility()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "TerminateAbility:%{public}s", abilityInfo_.name.c_str());

    // Notify all startAbilityByCall callers that this ability is terminating
    NotifyCallersOnTerminate();

#ifdef WITH_DLP
    HandleDlpClosed();
#endif // WITH_DLP
    AAFwk::EventInfo eventInfo;
    BuildTerminateAbilityEventInfo(eventInfo, 0);
    if (clearMissionFlag_) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "deleteAbilityRecoverInfo before clearMission");
        (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
            DeleteAbilityRecoverInfo(GetAbilityInfo().applicationInfo.accessTokenId, GetAbilityInfo().moduleName,
            GetAbilityInfo().name);
    }
    ResSchedUtil::GetInstance().ReportLoadingEventToRss(LoadingStage::DESTROY_END, GetPid(), GetUid(),
        0, GetRecordId());
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::TERMINATE_ABILITY, HISYSEVENT_BEHAVIOR, eventInfo);
    auto ret = DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(token_, clearMissionFlag_);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "terminate ability failed: %{public}d", ret);
    }
    return ret;
}

void AbilityRecord::BuildTerminateAbilityEventInfo(EventInfo &eventInfo, int32_t errCode)
{
    eventInfo.bundleName = GetAbilityInfo().bundleName;
    eventInfo.abilityName = GetAbilityInfo().name;
    eventInfo.appIndex = abilityInfo_.applicationInfo.appIndex;
    eventInfo.errCode = errCode;
}

void AbilityRecord::SendTerminateAbilityErrorEvent(int32_t errCode)
{
    EventInfo eventInfo;
    BuildTerminateAbilityEventInfo(eventInfo, errCode);
    EventReport::SendAbilityEvent(EventName::TERMINATE_ABILITY_ERROR, HISYSEVENT_FAULT, eventInfo);
}

const AppExecFwk::AbilityInfo &AbilityRecord::GetAbilityInfo() const
{
    return abilityInfo_;
}

const AppExecFwk::ApplicationInfo &AbilityRecord::GetApplicationInfo() const
{
    return abilityInfo_.applicationInfo;
}

AbilityState AbilityRecord::GetAbilityState() const
{
    return currentState_.load();
}

WindowConfig AbilityRecord::GetAbilityWindowConfig() const
{
    return windowConfig_;
}

bool AbilityRecord::IsForeground() const
{
    auto state = currentState_.load();
    return state == AbilityState::FOREGROUND || state == AbilityState::FOREGROUNDING;
}

AbilityVisibilityState AbilityRecord::GetAbilityVisibilityState() const
{
    return abilityVisibilityState_.load();
}

void AbilityRecord::SetAbilityVisibilityState(AbilityVisibilityState state)
{
    abilityVisibilityState_.store(state);
}

void AbilityRecord::UpdateAbilityVisibilityState()
{
    if (GetAbilityVisibilityState() == AbilityVisibilityState::INITIAL) {
        auto state = AbilityVisibilityState::UNSPECIFIED;
        auto sessionInfo = GetSessionInfo();
        if (sessionInfo && sessionInfo->processOptions &&
            (ProcessOptions::IsNewProcessMode(sessionInfo->processOptions->processMode) ||
             sessionInfo->processOptions->isStartFromNDK)) {
            auto startupVisibility = sessionInfo->processOptions->startupVisibility;
            if (startupVisibility == StartupVisibility::STARTUP_SHOW) {
                state = AbilityVisibilityState::FOREGROUND_SHOW;
            } else if (startupVisibility == StartupVisibility::STARTUP_HIDE) {
                state = AbilityVisibilityState::FOREGROUND_HIDE;
            }
        }
        SetAbilityVisibilityState(state);
    }
}
#ifdef SUPPORT_SCREEN
void AbilityRecord::SetAbilityStateInner(AbilityState state)
{
    currentState_.store(state);
    if (state == AbilityState::BACKGROUND) {
        isAbilityForegrounding_ = false;
    }

    auto collaborator = DelayedSingleton<AbilityManagerService>::GetInstance()->GetCollaborator(
        collaboratorType_);
    if (collaborator != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "notify collaborator, missionId:%{public}d, state:%{public}d", missionId_,
            static_cast<int32_t>(state));
        int ret = ERR_OK;
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            auto sessionInfo = GetSessionInfo();
            if (sessionInfo == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null sessionInfo");
                return;
            }
            int32_t persistentId = sessionInfo->persistentId;
            switch (state) {
                case AbilityState::BACKGROUNDING: {
                    ret = collaborator->NotifyMoveMissionToBackground(persistentId, GetOwnerMissionUserId());
                    break;
                }
                case AbilityState::TERMINATING: {
                    ret = collaborator->NotifyTerminateMission(persistentId, GetOwnerMissionUserId());
                    break;
                }
                default:
                    break;
            }
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed,err: %{public}d", ret);
            }
            return;
        }
        switch (state) {
            case AbilityState::FOREGROUNDING: {
                ret = collaborator->NotifyMoveMissionToForeground(missionId_, GetOwnerMissionUserId());
                break;
            }
            case AbilityState::BACKGROUNDING: {
                ret = collaborator->NotifyMoveMissionToBackground(missionId_, GetOwnerMissionUserId());
                break;
            }
            case AbilityState::TERMINATING: {
                ret = collaborator->NotifyTerminateMission(missionId_, GetOwnerMissionUserId());
                break;
            }
            default:
                break;
        }
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, err: %{public}d", ret);
        }
    }

    auto missionListWrap = DelayedSingleton<AbilityManagerService>::GetInstance()->GetMissionListWrap();
    if (missionListWrap != nullptr) {
        missionListWrap->SetMissionAbilityState(missionId_, state);
    }
}
#endif // SUPPORT_SCREEN
bool AbilityRecord::GetAbilityForegroundingFlag() const
{
    return isAbilityForegrounding_;
}

#ifdef SUPPORT_SCREEN
void AbilityRecord::SetAbilityState(AbilityState state)
{
    SetAbilityStateInner(state);
    if (state == AbilityState::FOREGROUND || state == AbilityState::ACTIVE || state == AbilityState::BACKGROUND) {
        SetRestarting(false);
    }
    if (state == AbilityState::FOREGROUND) {
        ResSchedUtil::GetInstance().ReportLoadingEventToRss(LoadingStage::FOREGROUND_END, GetPid(),
            GetUid(), 0, GetAbilityRecordId());
    }
}
#endif // SUPPORT_SCREEN
void AbilityRecord::SetScheduler(const sptr<IAbilityScheduler> &scheduler)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SetScheduler, bundlename: %{public}s, %{public}s",
        abilityInfo_.name.c_str(), abilityInfo_.applicationInfo.bundleName.c_str());
    CHECK_POINTER(lifecycleDeal_);
    if (scheduler != nullptr) {
        if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
            auto schedulerObject = scheduler_->AsObject();
            if (schedulerObject != nullptr) {
                schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
            }
        }
        if (schedulerDeathRecipient_ == nullptr) {
            schedulerDeathRecipient_ =
                new AbilitySchedulerRecipient([thisWeakPtr = weak_from_this()](const wptr<IRemoteObject> &remote) {
                    auto abilityRecord = thisWeakPtr.lock();
                    if (abilityRecord) {
                        abilityRecord->OnSchedulerDied(remote);
                    }
                });
        }
        isReady_ = true;
        scheduler_ = scheduler;
        lifecycleDeal_->SetScheduler(scheduler);
        auto schedulerObject = scheduler_->AsObject();
        if (schedulerObject == nullptr || !schedulerObject->AddDeathRecipient(schedulerDeathRecipient_)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "AddDeathRecipient failed");
        }
        SetPid(IPCSkeleton::GetCallingPid()); // set pid when ability attach to service.
        AfterLoaded();
        // add collaborator mission bind pid
        NotifyMissionBindPid();
#ifdef WITH_DLP
        HandleDlpAttached();
#endif // WITH_DLP
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null scheduler");
        isReady_ = false;
        isWindowAttached_ = false;
        SetIsNewWant(false);
        if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
            auto schedulerObject = scheduler_->AsObject();
            if (schedulerObject != nullptr) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "RemoveDeathRecipient");
                schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
            }
        }
        scheduler_ = scheduler;
        SetPid(0);
    }
}

void AbilityRecord::AfterLoaded()
{
    FreezeUtil::GetInstance().DeleteAppLifecycleEvent(GetPid());
    if (GetAbilityInfo().extensionAbilityType != AppExecFwk::ExtensionAbilityType::SERVICE) {
        ResSchedUtil::GetInstance().ReportLoadingEventToRss(LoadingStage::LOAD_END, GetPid(),
            GetUid(), 0, GetAbilityRecordId());
    }

    if (IsSceneBoard()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Sceneboard Added");
    }
}

sptr<Token> AbilityRecord::GetToken() const
{
    return token_;
}

bool AbilityRecord::IsReady() const
{
    return isReady_;
}

bool AbilityRecord::IsLauncherAbility() const
{
    return isLauncherAbility_;
}

bool AbilityRecord::IsTerminating() const
{
    return isTerminating_;
}

void AbilityRecord::SetTerminatingState()
{
    isTerminating_ = true;
}

bool AbilityRecord::IsNewWant() const
{
    return lifeCycleStateInfo_.isNewWant;
}

void AbilityRecord::SetIsNewWant(bool isNewWant)
{
    lifeCycleStateInfo_.isNewWant = isNewWant;
}

bool AbilityRecord::IsCreateByConnect() const
{
    return isCreateByConnect_;
}

void AbilityRecord::SetCreateByConnectMode(bool isCreatedByConnect)
{
    isCreateByConnect_ = isCreatedByConnect;
}

void AbilityRecord::Activate()
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "activate");
    CHECK_POINTER(lifecycleDeal_);

    if (!IsDebug()) {
        int activeTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * ACTIVE_TIMEOUT_MULTIPLE;
        SendEvent(AbilityManagerService::ACTIVE_TIMEOUT_MSG, activeTimeout);
    }

    // schedule active after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
#ifdef SUPPORT_SCREEN
    SetAbilityStateInner(AbilityState::ACTIVATING);
#endif // SUPPORT_SCREEN
    lifecycleDeal_->Activate(GetWant(), lifeCycleStateInfo_);
}

void AbilityRecord::Inactivate()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Inactivate:%{public}s.", abilityInfo_.name.c_str());
    CHECK_POINTER(lifecycleDeal_);

    if (!IsDebug()) {
        bool useOldMultiple = abilityInfo_.name == AbilityConfig::LAUNCHER_ABILITY_NAME ||
            abilityInfo_.name == AbilityConfig::CALLUI_ABILITY_NAME;
        auto timeoutMultiple = useOldMultiple ? INACTIVE_TIMEOUT_MULTIPLE : INACTIVE_TIMEOUT_MULTIPLE_NEW;
        auto inactiveTimeout =
            AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * timeoutMultiple;
        SendEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, inactiveTimeout);
    }

    // schedule inactive after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
#ifdef SUPPORT_SCREEN
    SetAbilityStateInner(AbilityState::INACTIVATING);
#endif // SUPPORT_SCREEN
    Want want = GetWant();
    UpdateDmsCallerInfo(want);
    lifecycleDeal_->Inactivate(want, lifeCycleStateInfo_, GetSessionInfo());
}

void AbilityRecord::Terminate(const Closure &task)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Terminate: %{public}s.", GetURI().c_str());
    CHECK_POINTER(lifecycleDeal_);
    if (!IsDebug()) {
        auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
        if (handler && task) {
            int terminateTimeout =
                AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * TERMINATE_TIMEOUT_MULTIPLE;
            if (abilityInfo_.applicationInfo.asanEnabled) {
                terminateTimeout =
                    AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * TERMINATE_TIMEOUT_ASANENABLED;
            }
            if (!isReady_) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "terminate with not ready");
                terminateTimeout = 0;
            }
            handler->SubmitTask(task, "terminate_" + std::to_string(recordId_), terminateTimeout);
            ResSchedUtil::GetInstance().ReportLoadingEventToRss(LoadingStage::DESTROY_BEGIN, GetPid(), GetUid(),
                terminateTimeout, GetRecordId());
        }
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "debug mode, no need to handle");
    }
#ifdef WITH_DLP
    HandleDlpClosed();
#endif // WITH_DLP
    // schedule background after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
#ifdef SUPPORT_SCREEN
    SetAbilityStateInner(AbilityState::TERMINATING);
#endif // SUPPORT_SCREEN
    lifecycleDeal_->Terminate(GetWant(), lifeCycleStateInfo_, GetSessionInfo());

    ReportAbilityConnectionForResultTerminate();
    if (GetCallerInfo() != nullptr && UIExtensionWrapper::IsUIExtension(GetAbilityInfo().extensionAbilityType)) {
        DelayedSingleton<ForegroundAppConnectionManager>::GetInstance()->AbilityRemovePidConnection(
            GetCallerInfo()->callerPid, GetPid(), GetAbilityRecordId());
    }
}

void AbilityRecord::ShareData(const int32_t &uniqueId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ShareData:%{public}s", abilityInfo_.name.c_str());
    CHECK_POINTER(lifecycleDeal_);
    if (!IsDebug()) {
        int loadTimeout =
            AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * SHAREDATA_TIMEOUT_MULTIPLE;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "loadTimeOut %{public}d.", loadTimeout);
        SendEvent(AbilityManagerService::SHAREDATA_TIMEOUT_MSG, loadTimeout, uniqueId);
    }
    lifecycleDeal_->ShareData(uniqueId);
}

void AbilityRecord::CommandAbility()
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "startId_:%{public}d.", startId_);
    Want want = GetWant();
    UpdateDmsCallerInfo(want);
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->CommandAbility(want, false, startId_);
}

void AbilityRecord::CommandAbilityWindow(const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd)
{
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->CommandAbilityWindow(GetWant(), sessionInfo, winCmd);
}

void AbilityRecord::SaveAbilityState()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->SaveAbilityState();
}

void AbilityRecord::SaveAbilityState(const PacMap &inState)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    stateDatas_ = inState;
}

void AbilityRecord::SaveAbilityWindowConfig(const WindowConfig &windowConfig)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    windowConfig_ = windowConfig;
}

void AbilityRecord::RestoreAbilityState()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->RestoreAbilityState(stateDatas_);
    stateDatas_.Clear();
    isRestarting_ = false;
}

int AbilityRecord::GetRequestCode() const
{
    return requestCode_;
}

void AbilityRecord::SetResult(const std::shared_ptr<AbilityResult> &result)
{
    std::lock_guard guard(resultLock_);
    result_ = result;
}

std::shared_ptr<AbilityResult> AbilityRecord::GetResult() const
{
    std::lock_guard guard(resultLock_);
    return result_;
}

void AbilityRecord::SendResult(bool isSandboxApp, uint32_t tokeId, bool schedulerdied)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SendResult:%{public}s,tokeId:%{public}u,%{public}d",
        abilityInfo_.name.c_str(), tokeId, static_cast<int32_t>(schedulerdied));
    std::lock_guard<ffrt::mutex> guard(lock_);
    CHECK_POINTER(scheduler_);
    auto result = GetResult();
    CHECK_POINTER(result);
#ifdef SUPPORT_UPMS
    if (!schedulerdied) {
        GrantUriPermission(result->resultWant_, abilityInfo_.applicationInfo.bundleName, isSandboxApp, tokeId, false);
    }
#endif // SUPPORT_UPMS
    if (result->requestCode_ != -1) {
        scheduler_->SendResult(result->requestCode_, result->resultCode_, result->resultWant_);
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "requestCode_ -1");
    }
    // reset result to avoid send result next time
    SetResult(nullptr);
}

void AbilityRecord::SendResultByBackToCaller(const std::shared_ptr<AbilityResult> &result)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SendResultByBackToCaller:%{public}s", abilityInfo_.name.c_str());
    std::lock_guard<ffrt::mutex> guard(lock_);
    CHECK_POINTER(scheduler_);
    CHECK_POINTER(result);
    scheduler_->SendResult(result->requestCode_, result->resultCode_, result->resultWant_);
    if (GetCallerInfo() != nullptr) {
        DelayedSingleton<ForegroundAppConnectionManager>::GetInstance()->AbilityRemovePidConnection(
            GetCallerInfo()->callerPid, GetPid(), GetAbilityRecordId());
    }
}

void AbilityRecord::SendSandboxSavefileResult(const Want &want, int resultCode, int requestCode)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SendSandboxSavefileResult:%{public}s", abilityInfo_.name.c_str());

    auto uriParam = want.GetParams().GetParam(PARAMS_FILE_SAVING_URL_KEY);
    auto uriArray = AAFwk::IArray::Query(uriParam);
    long arraySize = 0;
    if (uriArray && uriArray->GetLength(arraySize) == ERR_OK &&
        arraySize > 0 && AAFwk::Array::IsStringArray(uriArray)) {
        for (long i = 0; i < arraySize; i++) {
            sptr<AAFwk::IInterface> iface = nullptr;
            if (uriArray->Get(i, iface) != ERR_OK) {
                continue;
            }
            AAFwk::IString* iuri = AAFwk::IString::Query(iface);
            if (!iuri) {
                continue;
            }
            std::string uriStr;
            if (iuri->GetString(uriStr) != ERR_OK) {
                continue;
            }
            Uri uri(uriStr);
            uint32_t initiatorTokenId = IPCSkeleton::GetCallingTokenID();
#ifdef SUPPORT_UPMS
            auto flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION | Want::FLAG_AUTH_READ_URI_PERMISSION;
            auto ret = IN_PROCESS_CALL(UriPermissionManagerClient::GetInstance().GrantUriPermission(uri,
                flag, abilityInfo_.bundleName, appIndex_, initiatorTokenId));
            if (ret != ERR_OK) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "GrantUriPermission failed");
            }
#endif // SUPPORT_UPMS
        }
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "uri illegal for request: %{public}d", requestCode);
    }

    auto scheduler = scheduler_;
    if (scheduler) {
        scheduler->SendResult(requestCode, resultCode, want);
    }
}

void AbilityRecord::SendResultToCallers(bool schedulerdied)
{
    for (auto caller : GetCallerRecordList()) {
        if (caller == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null caller");
            continue;
        }
        std::shared_ptr<AbilityRecord> callerAbilityRecord = caller->GetCaller();
        if (callerAbilityRecord != nullptr && callerAbilityRecord->GetResult() != nullptr) {
            bool isSandboxApp = appIndex_ > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX ? true : false;
            callerAbilityRecord->SendResult(isSandboxApp, abilityInfo_.applicationInfo.accessTokenId, schedulerdied);
        } else {
            std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord = caller->GetSaCaller();
            if (callerSystemAbilityRecord != nullptr) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "send result to system ability");
                callerSystemAbilityRecord->SendResultToSystemAbility(caller->GetRequestCode(),
                    callerSystemAbilityRecord, abilityInfo_.applicationInfo.uid,
                    abilityInfo_.applicationInfo.accessTokenId, schedulerdied);
            }
        }
    }
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetCallerByRequestCode(int32_t requestCode, int32_t pid)
{
    for (auto caller : GetCallerRecordList()) {
        if (caller == nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "null caller");
            continue;
        }
        std::shared_ptr<AbilityRecord> callerAbilityRecord = caller->GetCaller();
        if (callerAbilityRecord == nullptr || callerAbilityRecord->GetPid() != pid) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "callerAbility not match");
            continue;
        }
        if (caller->IsHistoryRequestCode(requestCode)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "found callerAbility");
            return callerAbilityRecord;
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Can't found caller");
    return nullptr;
}

void AbilityRecord::SaveResultToCallers(const int resultCode, const Want *resultWant)
{
    auto callerRecordList = GetCallerRecordList();
    if (callerRecordList.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callerRecordList empty");
        return;
    }
    auto latestCaller = callerRecordList.back();
    for (auto caller : callerRecordList) {
        if (caller == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null caller");
            continue;
        }
        if (caller == latestCaller) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "latestCaller");
            SaveResult(resultCode, resultWant, caller);
            continue;
        }
        SaveResult(SEND_RESULT_CANCELED, resultWant, caller);
    }
}

void AbilityRecord::SaveResult(int resultCode, const Want *resultWant, std::shared_ptr<CallerRecord> caller)
{
    std::lock_guard<ffrt::mutex> guard(lock_);
    std::shared_ptr<AbilityRecord> callerAbilityRecord = caller->GetCaller();
    if (callerAbilityRecord != nullptr) {
        Want* newWant = const_cast<Want*>(resultWant);
        if (callerAbilityRecord->GetApplicationInfo().name == AppUtils::GetInstance().GetBrokerDelegateBundleName()) {
            newWant->SetParam(std::string(PARAM_SEND_RESULT_CALLER_BUNDLENAME), abilityInfo_.applicationInfo.name);
            newWant->SetParam(std::string(PARAM_SEND_RESULT_CALLER_TOKENID), static_cast<int32_t>(
                abilityInfo_.applicationInfo.accessTokenId));
        }
        callerAbilityRecord->SetResult(
            std::make_shared<AbilityResult>(caller->GetRequestCode(), resultCode, *newWant));
    } else {
        std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord = caller->GetSaCaller();
        if (callerSystemAbilityRecord != nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "caller is system ability");
            Want* newWant = const_cast<Want*>(resultWant);
            if (want_.GetBoolParam(PARAM_RESV_ANCO_IS_NEED_UPDATE_NAME, false)) {
                want_.RemoveParam(PARAM_RESV_ANCO_IS_NEED_UPDATE_NAME);
                callerSystemAbilityRecord->SetResult(*newWant, resultCode);
            } else {
                callerSystemAbilityRecord->SetResultToSystemAbility(callerSystemAbilityRecord, *newWant,
                    resultCode);
            }
        }
    }
}

void SystemAbilityCallerRecord::SetResultToSystemAbility(
    std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord,
    Want &resultWant, int resultCode)
{
    std::vector<std::string> data;
    std::string srcAbilityId = callerSystemAbilityRecord->GetSrcAbilityId();
    SplitStr(srcAbilityId, "_", data);
    if (data.size() != VECTOR_SIZE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "check size failed");
        return;
    }
    std::string srcDeviceId = data[0];
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get srcDeviceId = %{public}s", srcDeviceId.c_str());
    int missionId = atoi(data[1].c_str());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "get missionId: %{public}d", missionId);
    resultWant.SetParam(DMS_SRC_NETWORK_ID, srcDeviceId);
    resultWant.SetParam(DMS_MISSION_ID, missionId);
    callerSystemAbilityRecord->SetResult(resultWant, resultCode);
}

void SystemAbilityCallerRecord::SendResultToSystemAbility(int requestCode,
    const std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord,
    int32_t callerUid, uint32_t accessToken, bool schedulerdied)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    if (callerSystemAbilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null record");
        return;
    }
    int resultCode = callerSystemAbilityRecord->GetResultCode();
    Want resultWant = callerSystemAbilityRecord->GetResultWant();
    sptr<IRemoteObject> callerToken = callerSystemAbilityRecord->GetCallerToken();
    if (!schedulerdied) {
        callerUid = IPCSkeleton::GetCallingUid();
        accessToken = IPCSkeleton::GetCallingTokenID();
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Try to SendResult");
    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null callerToken");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SYSTEM_ABILITY_TOKEN_CALLBACK)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }
    if (!data.WriteParcelable(&resultWant)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "writeParcelable failed");
        return;
    }
    if (!data.WriteInt32(callerUid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write callerUid failed");
        return;
    }
    if (!data.WriteInt32(requestCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write requestCode failed");
        return;
    }
    if (!data.WriteUint32(accessToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write accessToken failed");
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write resultCode failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int result = callerToken->SendRequest(ISystemAbilityTokenCallback::SEND_RESULT, data, reply, option);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", result);
    }
}

void AbilityRecord::RemoveSpecifiedWantParam(const std::string &key)
{
    std::lock_guard guard(wantLock_);
    if (want_.HasParameter(key)) {
        want_.RemoveParam(key);
    }
}

void AbilityRecord::RemoveCallerRequestCode(std::shared_ptr<AbilityRecord> callerAbilityRecord, int32_t requestCode)
{
    if (callerAbilityRecord == nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "null record");
        return;
    }
    std::lock_guard guard(callerListLock_);
    for (auto it = callerList_.begin(); it != callerList_.end(); it++) {
        if ((*it)->GetCaller() == callerAbilityRecord) {
            (*it)->RemoveHistoryRequestCode(requestCode);
            if ((*it)->GetRequestCodeSet().empty()) {
                callerList_.erase(it);
                TAG_LOGI(AAFwkTag::ABILITYMGR, "remove callerRecord");
            }
            return;
        }
    }
}

void AbilityRecord::AddCallerRecord(const sptr<IRemoteObject> &callerToken, int requestCode, const Want &want,
    std::string srcAbilityId, uint32_t callingTokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call, callingTokenId:%{public}u,requestCode:%{public}d",
        callingTokenId, requestCode);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        RecordSaCallerInfo(want);
    }
    if (!srcAbilityId.empty() && IsSystemAbilityCall(callerToken, callingTokenId)) {
        AddSystemAbilityCallerRecord(callerToken, requestCode, srcAbilityId);
        return;
    }

    if (!abilityRecord) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "nullptr");
        return;
    }

    auto isExist = [&abilityRecord](const std::shared_ptr<CallerRecord> &callerRecord) {
        return (callerRecord->GetCaller() == abilityRecord);
    };
    std::lock_guard guard(callerListLock_);
    auto record = std::find_if(callerList_.begin(), callerList_.end(), isExist);
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, abilityRecord);
    if (record != callerList_.end()) {
        newCallerRecord->SetRequestCodeSet((*record)->GetRequestCodeSet());
        callerList_.erase(record);
    }
    newCallerRecord->AddHistoryRequestCode(requestCode);
    callerList_.emplace_back(newCallerRecord);

    lifeCycleStateInfo_.caller.requestCode = requestCode;
    lifeCycleStateInfo_.caller.deviceId = abilityRecord->GetAbilityInfo().deviceId;
    lifeCycleStateInfo_.caller.bundleName = abilityRecord->GetAbilityInfo().bundleName;
    lifeCycleStateInfo_.caller.abilityName = abilityRecord->GetAbilityInfo().name;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "caller %{public}s, %{public}s, callerSize: %{public}zu",
        abilityRecord->GetAbilityInfo().bundleName.c_str(),
        abilityRecord->GetAbilityInfo().name.c_str(), callerList_.size());
}

bool AbilityRecord::IsSystemAbilityCall(const sptr<IRemoteObject> &callerToken, uint32_t callingTokenId)
{
    if (callerToken == nullptr) {
        return false;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord != nullptr) {
        return false;
    }
    uint32_t tokenId = 0;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        tokenId = callingTokenId;
    } else {
        tokenId = IPCSkeleton::GetCallingTokenID();
    }
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    bool isNativeCall = tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    if (!isNativeCall) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not native call");
        return false;
    }
    AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, nativeTokenInfo);
    if (result == ERR_OK && (nativeTokenInfo.processName == DMS_PROCESS_NAME ||
        want_.GetBoolParam(PARAM_RESV_ANCO_IS_NEED_UPDATE_NAME, false))) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "system ability call");
        return true;
    }
    return false;
}

void AbilityRecord::AddSystemAbilityCallerRecord(const sptr<IRemoteObject> &callerToken, int requestCode,
    std::string srcAbilityId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "add system ability caller record");
    std::shared_ptr<SystemAbilityCallerRecord> systemAbilityRecord =
        std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, callerToken);
    auto isExist = [&srcAbilityId](const std::shared_ptr<CallerRecord> &callerRecord) {
        std::shared_ptr<SystemAbilityCallerRecord> saCaller = callerRecord->GetSaCaller();
        return (saCaller != nullptr && saCaller->GetSrcAbilityId() == srcAbilityId);
    };
    std::lock_guard guard(callerListLock_);
    auto record = std::find_if(callerList_.begin(), callerList_.end(), isExist);
    if (record != callerList_.end()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "find same system ability caller record");
        callerList_.erase(record);
    }
    callerList_.emplace_back(std::make_shared<CallerRecord>(requestCode, systemAbilityRecord));
    TAG_LOGI(AAFwkTag::ABILITYMGR, "add system ability record end");
}

void AbilityRecord::RecordSaCallerInfo(const Want &want)
{
    saCallerInfo_ = std::make_shared<CallerAbilityInfo>();
    saCallerInfo_->callerTokenId = want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0);
    saCallerInfo_->callerUid =  want.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0);
    saCallerInfo_->callerPid =  want.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0);
    saCallerInfo_->callerNativeName = want.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME);
}

std::list<std::shared_ptr<CallerRecord>> AbilityRecord::GetCallerRecordList() const
{
    std::lock_guard guard(callerListLock_);
    return callerList_;
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetCallerRecord() const
{
    std::lock_guard guard(callerListLock_);
    if (callerList_.empty()) {
        return nullptr;
    }
    if (callerList_.back() == nullptr) {
        return nullptr;
    }
    return callerList_.back()->GetCaller();
}

std::shared_ptr<CallerAbilityInfo> AbilityRecord::GetCallerInfo() const
{
    std::lock_guard guard(callerListLock_);
    if (callerList_.empty() || callerList_.back() == nullptr) {
        return saCallerInfo_;
    }
    return callerList_.back()->GetCallerInfo();
}

void AbilityRecord::GetAbilityTypeString(std::string &typeStr)
{
    AppExecFwk::AbilityType type = GetAbilityInfo().type;
    switch (type) {
#ifdef SUPPORT_GRAPHICS
        case AppExecFwk::AbilityType::PAGE: {
            typeStr = "PAGE";
            break;
        }
#endif
        case AppExecFwk::AbilityType::SERVICE: {
            typeStr = UIExtensionWrapper::IsUIExtension(GetAbilityInfo().extensionAbilityType) ?
                "UIEXTENSION" : "SERVICE";
            break;
        }
        // for config.json type
        case AppExecFwk::AbilityType::DATA: {
            typeStr = "DATA";
            break;
        }
        default: {
            typeStr = "UNKNOWN";
            break;
        }
    }
}

std::string AbilityRecord::ConvertAbilityState(const AbilityState &state)
{
    return  StateUtils::StateToStrMap(state);
}

std::string AbilityRecord::ConvertAppState(const AppState &state)
{
    return StateUtils::AppStateToStrMap(state);
}

int AbilityRecord::ConvertLifeCycleToAbilityState(const AbilityLifeCycleState &state)
{
    return StateUtils::ConvertStateMap(state);
}

void AbilityRecord::Dump(std::vector<std::string> &info)
{
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    std::string isKeepAlive = GetKeepAlive() ? "true" : "false";
    dumpInfo = "        isKeepAlive: " + isKeepAlive;
    info.push_back(dumpInfo);
    // get ability type(unknown/page/service/provider)
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);

    dumpInfo = "        previous ability app name [NULL]";
    dumpInfo.append("\n");
    dumpInfo += "        previous ability file name [NULL]";
    info.push_back(dumpInfo);

    dumpInfo = "        next ability app name [NULL]";
    dumpInfo.append("\n");
    dumpInfo += "        next ability file name [NULL]";
    info.push_back(dumpInfo);
    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);

    if (isLauncherRoot_) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }
}

void AbilityRecord::DumpAbilityState(
    std::vector<std::string> &info, bool isClient, const std::vector<std::string> &params)
{
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);

    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);
    dumpInfo = "        callee connections: ";
    info.push_back(dumpInfo);
    if (callContainer_) {
        callContainer_->Dump(info);
    }

    std::string isKeepAlive = GetKeepAlive() ? "true" : "false";
    dumpInfo = "        isKeepAlive: " + isKeepAlive;
    info.push_back(dumpInfo);
    if (isLauncherRoot_) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }

    auto missionAffinity = GetMissionAffinity();
    if (!missionAffinity.empty()) {
        dumpInfo = "        missionAffinity: " + missionAffinity;
        info.push_back(dumpInfo);
    }

    // add dump client info
    DumpClientInfo(info, params, isClient, params.empty());
}

void AbilityRecord::SetStartTime()
{
    if (startTime_ == 0) {
        startTime_ = AbilityUtil::SystemTimeMillis();
    }
}

int64_t AbilityRecord::GetStartTime() const
{
    return startTime_;
}

void AbilityRecord::RemoveAbilityDeathRecipient() const
{
    if (scheduler_ == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null scheduler_");
        return;
    }

    if (schedulerDeathRecipient_ == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null schedulerDeathRecipient_");
        return;
    }

    auto schedulerObject = scheduler_->AsObject();
    if (schedulerObject != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "RemoveDeathRecipient");
        schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
    }
}

void AbilityRecord::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnSchedulerDied called");
    std::lock_guard<ffrt::mutex> guard(lock_);
    if (IsAbilityDiedHandled()) {
        return;
    }
    CHECK_POINTER(scheduler_);

    auto object = remote.promote();
    CHECK_POINTER(object);

    if (object != scheduler_->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability died: scheduler not matches remote");
        return;
    }

    if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObject = scheduler_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->SetScheduler(nullptr);
    isWindowAttached_ = false;

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER(handler);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Ability on scheduler died: '%{public}s'", abilityInfo_.name.c_str());
    auto task = [ability = shared_from_this()]() {
        CHECK_POINTER_LOG(ability, "ability nullptr");
        DelayedSingleton<AbilityManagerService>::GetInstance()->OnAbilityDied(ability);
    };
    handler->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = "OnSchedulerDied",
        .timeoutMillis_ = SCHEDULER_DIED_TIMEOUT
    });
    auto resultTask = [want = GetWant(), ability = shared_from_this()]() {
        ability->SaveResultToCallers(-1, &want);
        ability->SendResultToCallers(true);
    };
    handler->SubmitTask(resultTask);
#ifdef SUPPORT_GRAPHICS
    NotifyAnimationAbilityDied();
#endif
#ifdef WITH_DLP
    HandleDlpClosed();
#endif // WITH_DLP
    NotifyRemoveShellProcess(CollaboratorType::RESERVE_TYPE);
    NotifyRemoveShellProcess(CollaboratorType::OTHERS_TYPE);
}

bool AbilityRecord::IsAbilityDiedHandled()
{
    if (handledAbilityDied_) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityDied already handled");
        return handledAbilityDied_;
    }
    handledAbilityDied_ = true;
    return false;
}

void AbilityRecord::OnProcessDied(bool isKeepAliveDied)
{
    CancelPrepareTerminate();
    std::lock_guard<ffrt::mutex> guard(lock_);
    if (!IsSceneBoard() && scheduler_ != nullptr && !isKeepAliveDied) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnProcessDied: '%{public}s', attached.", abilityInfo_.name.c_str());
        return;
    }
    if (IsAbilityDiedHandled()) {
        return;
    }
    if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObject = scheduler_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
    if (lifecycleDeal_ != nullptr) {
        lifecycleDeal_->SetScheduler(nullptr);
    }
    isWindowAttached_ = false;

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER(handler);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnProcessDied: '%{public}s'", abilityInfo_.name.c_str());
    auto task = [ability = shared_from_this()]() {
        CHECK_POINTER_LOG(ability, "ability nullptr");
        DelayedSingleton<AbilityManagerService>::GetInstance()->OnAbilityDied(ability);
    };
    if (IsSceneBoard()) {
        handler->SubmitTask(task, RESTART_SCENEBOARD_DELAY);
    } else {
        handler->SubmitTask(task);
    }
    auto uriTask = [want = GetWant(), ability = shared_from_this()]() {
        ability->SaveResultToCallers(-1, &want);
        ability->SendResultToCallers(true);
    };
    handler->SubmitTask(uriTask);
#ifdef SUPPORT_GRAPHICS
    NotifyAnimationAbilityDied();
#endif
#ifdef WITH_DLP
    HandleDlpClosed();
#endif // WITH_DLP
    NotifyRemoveShellProcess(CollaboratorType::RESERVE_TYPE);
    NotifyRemoveShellProcess(CollaboratorType::OTHERS_TYPE);
}

void AbilityRecord::NotifyAnimationAbilityDied()
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    // notify winddow manager service the ability died
    if (missionId_ != -1) {
#ifdef SUPPORT_SCREEN
        if (GetWMSHandler()) {
            sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
            SetAbilityTransitionInfo(info);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "UIAbiltiy abnormal death");
            GetWMSHandler()->NotifyAnimationAbilityDied(info);
        }
#endif // SUPPORT_SCREEN
    }
}

bool AbilityRecord::IsNeverStarted() const
{
    if (UIExtensionWrapper::IsUIExtension(GetAbilityInfo().extensionAbilityType)) {
        return startId_ == 0;
    }
    return startId_ == 0 && IsCreateByConnect();
}

void AbilityRecord::AddStartId()
{
    startId_++;
}
int AbilityRecord::GetStartId() const
{
    return startId_;
}

void AbilityRecord::SetIsUninstallAbility()
{
    isUninstall_ = true;
}

bool AbilityRecord::IsUninstallAbility() const
{
    return isUninstall_;
}

void AbilityRecord::SetLauncherRoot()
{
    isLauncherRoot_ = true;
}

bool AbilityRecord::IsLauncherRoot() const
{
    return isLauncherRoot_;
}

bool AbilityRecord::IsAbilityState(const AbilityState &state) const
{
    return (currentState_.load() == state);
}

bool AbilityRecord::IsActiveState() const
{
    return (IsAbilityState(AbilityState::ACTIVE) || IsAbilityState(AbilityState::ACTIVATING) ||
            IsAbilityState(AbilityState::INITIAL) || IsAbilityState(AbilityState::FOREGROUND) ||
            IsAbilityState(AbilityState::FOREGROUNDING));
}

void AbilityRecord::SendEvent(uint32_t msg, uint32_t timeOut, int32_t param, bool isExtension,
    const std::string &taskName)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER(handler);
    param = (param == -1) ? recordId_ : param;
    auto eventWrap = EventWrap(msg, param, isExtension, taskName);
    eventWrap.SetTimeout(timeOut);
    if (!handler->SendEvent(eventWrap, timeOut, false)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "failed: %{public}u, %{public}d", msg, param);
    }
}

void AbilityRecord::SetWant(const Want &want)
{
    std::lock_guard guard(wantLock_);
    if (abilityInfo_.type == AppExecFwk::AbilityType::PAGE && isLaunching_) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "uIAbility launching");
        return;
    }
    auto multiThread = want_.GetBoolParam(MULTI_THREAD, false);
    auto errorInfoEnhance = want_.GetBoolParam(ERROR_INFO_ENHANCE, false);
    want_.CloseAllFd();

    want_ = want;
    if (launchDebugInfo_.isDebugAppSet) {
        want_.SetParam(AbilityConfig::DEBUG_APP, launchDebugInfo_.debugApp);
    } else {
        want_.RemoveParam(AbilityConfig::DEBUG_APP);
    }
    if (launchDebugInfo_.isNativeDebugSet) {
        want_.SetParam(AbilityConfig::NATIVE_DEBUG, launchDebugInfo_.nativeDebug);
    } else {
        want_.RemoveParam(AbilityConfig::NATIVE_DEBUG);
    }
    if (launchDebugInfo_.isPerfCmdSet) {
        want_.SetParam(AbilityConfig::PERF_CMD, launchDebugInfo_.perfCmd);
    } else {
        want_.RemoveParam(AbilityConfig::PERF_CMD);
    }
    if (multiThread) {
        want_.SetParam(MULTI_THREAD, true);
    }
    if (errorInfoEnhance) {
        want_.SetParam(ERROR_INFO_ENHANCE, true);
    }
    if (want_.HasParameter(UISERVICEHOSTPROXY_KEY)) {
        want_.RemoveParam(UISERVICEHOSTPROXY_KEY);
    }
    if (want_.HasParameter(AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY)) {
        want_.RemoveParam(AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY);
    }
}

Want AbilityRecord::GetWant() const
{
    std::lock_guard guard(wantLock_);
    return want_;
}

void AbilityRecord::RemoveSignatureInfo()
{
    std::lock_guard guard(wantLock_);
    want_.RemoveParam(Want::PARAM_RESV_CALLER_APP_ID);
    want_.RemoveParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER);
}

AppExecFwk::ElementName AbilityRecord::GetElementName() const
{
    std::lock_guard guard(wantLock_);
    return want_.GetElement();
}

bool AbilityRecord::IsDebugApp() const
{
    std::lock_guard guard(wantLock_);
    return launchDebugInfo_.debugApp;
}

bool AbilityRecord::IsDebug() const
{
    std::lock_guard guard(wantLock_);
    if (launchDebugInfo_.debugApp || launchDebugInfo_.nativeDebug ||
        !launchDebugInfo_.perfCmd.empty() || isAttachDebug_ || isAssertDebug_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "debug mode, no need to handle");
        return true;
    }
    return false;
}

void AbilityRecord::SetWindowMode(int32_t windowMode)
{
    std::lock_guard guard(wantLock_);
    want_.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
}

void AbilityRecord::RemoveWindowMode()
{
    std::lock_guard guard(wantLock_);
    want_.RemoveParam(Want::PARAM_RESV_WINDOW_MODE);
}

void AbilityRecord::UpdateRecoveryInfo(bool hasRecoverInfo)
{
    if (hasRecoverInfo) {
        std::lock_guard guard(wantLock_);
        want_.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
        SetLaunchReason(LaunchReason::LAUNCHREASON_APP_RECOVERY);
    }
}

bool AbilityRecord::GetRecoveryInfo()
{
    std::lock_guard guard(wantLock_);
    return want_.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false);
}

void AbilityRecord::SetStartSetting(const std::shared_ptr<AbilityStartSetting> &setting)
{
    lifeCycleStateInfo_.setting = setting;
}

std::shared_ptr<AbilityStartSetting> AbilityRecord::GetStartSetting() const
{
    return lifeCycleStateInfo_.setting;
}

void AbilityRecord::SetRestarting(const bool isRestart)
{
    isRestarting_ = isRestart;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SetRestarting: %{public}d", isRestarting_);
    if ((isLauncherRoot_ && IsLauncherAbility()) || GetKeepAlive()) {
        restartCount_ = isRestart ? (--restartCount_) : restartMax_;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "root launcher or resident process's restart count: %{public}d", restartCount_);
    }
}

void AbilityRecord::SetRestarting(const bool isRestart, int32_t canRestartCount)
{
    isRestarting_ = isRestart;
    TAG_LOGD(
        AAFwkTag::ABILITYMGR, "SetRestarting: %{public}d, restart count: %{public}d", isRestarting_, canRestartCount);

    if ((isLauncherRoot_ && IsLauncherAbility()) || GetKeepAlive()) {
        restartCount_ = isRestart ? canRestartCount : restartMax_;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "root launcher or resident process's restart count: %{public}d", restartCount_);
    }
}

int32_t AbilityRecord::GetRestartCount() const
{
    return restartCount_;
}

void AbilityRecord::SetRestartCount(int32_t restartCount)
{
    restartCount_ = restartCount;
}

bool AbilityRecord::IsRestarting() const
{
    return isRestarting_;
}

bool AbilityRecord::GetKeepAlive() const
{
    // Special ability
    std::vector<std::pair<std::string, std::string>> trustAbilities{
        { AbilityConfig::SCENEBOARD_BUNDLE_NAME, AbilityConfig::SCENEBOARD_ABILITY_NAME },
        { AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_ABILITY_NAME },
        { AbilityConfig::LAUNCHER_BUNDLE_NAME, AbilityConfig::LAUNCHER_ABILITY_NAME }
    };
    for (const auto &pair : trustAbilities) {
        if (pair.first == abilityInfo_.bundleName && pair.second == abilityInfo_.name) {
            return true;
        }
    }
    bool keepAliveEnable = keepAliveBundle_;
    AmsResidentProcessRdb::GetInstance().GetResidentProcessEnable(
        abilityInfo_.applicationInfo.bundleName, keepAliveEnable);
    return keepAliveEnable;
}

void AbilityRecord::SetLoading(bool status)
{
    isLoading_ = status;
}

bool AbilityRecord::IsLoading() const
{
    return isLoading_;
}

int64_t AbilityRecord::GetRestartTime()
{
    return restartTime_;
}

void AbilityRecord::SetRestartTime(const int64_t restartTime)
{
    restartTime_ = restartTime;
}

void AbilityRecord::SetAppState(const AppState &state)
{
    appState_ = state;
}

AppState AbilityRecord::GetAppState() const
{
    return appState_;
}

void AbilityRecord::SetLaunchReason(const LaunchReason &reason)
{
    if (isAppAutoStartup_) {
        lifeCycleStateInfo_.launchParam.launchReason = LaunchReason::LAUNCHREASON_AUTO_STARTUP;
        return;
    }
    lifeCycleStateInfo_.launchParam.launchReason = reason;
}

void AbilityRecord::SetLaunchReasonMessage(const std::string &launchReasonMessage)
{
    lifeCycleStateInfo_.launchParam.launchReasonMessage = launchReasonMessage;
}

void AbilityRecord::SetLastExitReason(const ExitReason &exitReason, const AppExecFwk::RunningProcessInfo &processInfo,
    const int64_t timestamp, bool withKillMsg)
{
    std::string exitMsg = exitReason.exitMsg;
    std::string killMsg = "";
    if (withKillMsg) {
        killMsg = exitReason.exitMsg;
    }
    LastExitDetailInfo lastExitDetailInfo = {};
    lastExitDetailInfo.pid = processInfo.pid_;
    lastExitDetailInfo.uid = processInfo.uid_;
    lastExitDetailInfo.exitSubReason = exitReason.subReason;
    lastExitDetailInfo.rss = processInfo.rssValue;
    lastExitDetailInfo.pss = processInfo.pssValue;
    lastExitDetailInfo.processState = static_cast<int32_t>(processInfo.state_);
    lastExitDetailInfo.timestamp = timestamp;
    lastExitDetailInfo.processName = processInfo.processName_;
    lastExitDetailInfo.exitMsg = killMsg;
    lifeCycleStateInfo_.launchParam.lastExitReason = CovertAppExitReasonToLastReason(exitReason.reason);
    lifeCycleStateInfo_.launchParam.lastExitMessage = exitMsg;
    lifeCycleStateInfo_.launchParam.lastExitDetailInfo = lastExitDetailInfo;
}

LastExitReason AbilityRecord::CovertAppExitReasonToLastReason(const Reason exitReason)
{
    switch (exitReason) {
        case REASON_NORMAL:
            return LASTEXITREASON_NORMAL;
        case REASON_CPP_CRASH:
            return LASTEXITREASON_CPP_CRASH;
        case REASON_JS_ERROR:
            return LASTEXITREASON_JS_ERROR;
        case REASON_APP_FREEZE:
            return LASTEXITREASON_APP_FREEZE;
        case REASON_PERFORMANCE_CONTROL:
            return LASTEXITREASON_PERFORMANCE_CONTROL;
        case REASON_RESOURCE_CONTROL:
            return LASTEXITREASON_RESOURCE_CONTROL;
        case REASON_UPGRADE:
            return LASTEXITREASON_UPGRADE;
        case REASON_USER_REQUEST:
            return LASTEXITREASON_USER_REQUEST;
        case REASON_SIGNAL:
            return LASTEXITREASON_SIGNAL;
        case REASON_UNKNOWN:
        default:
            return LASTEXITREASON_UNKNOWN;
    }
}

void AbilityRecord::NotifyContinuationResult(int32_t result)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyContinuationResult");
    CHECK_POINTER(lifecycleDeal_);

    lifecycleDeal_->NotifyContinuationResult(result);
}

void AbilityRecord::SetMissionId(int32_t missionId)
{
    std::lock_guard guard(wantLock_);
    missionId_ = missionId;
    want_.RemoveParam(KEY_MISSION_ID);
    want_.SetParam(KEY_MISSION_ID, missionId_);
}

void AbilityRecord::SetSessionInfo(sptr<SessionInfo> sessionInfo)
{
    std::lock_guard guard(sessionLock_);
    sessionInfo_ = sessionInfo;
}

sptr<SessionInfo> AbilityRecord::GetSessionInfo() const
{
    std::lock_guard guard(sessionLock_);
    return sessionInfo_;
}

void AbilityRecord::UpdateSessionInfo(sptr<IRemoteObject> sessionToken)
{
    {
        std::lock_guard guard(sessionLock_);
        if (sessionInfo_ == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null sessionInfo_");
            return;
        }
        sessionInfo_->sessionToken = sessionToken;
    }
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->UpdateSessionToken(sessionToken);
}

void AbilityRecord::SetMinimizeReason(bool fromUser)
{
    minimizeReason_ = fromUser;
}

void AbilityRecord::SetSceneFlag(uint32_t sceneFlag)
{
    lifeCycleStateInfo_.sceneFlag = sceneFlag;
}

void AbilityRecord::SetAppIndex(const int32_t appIndex)
{
    appIndex_ = appIndex;
}

int32_t AbilityRecord::GetAppIndex() const
{
    return appIndex_;
}

void AbilityRecord::SetWantAppIndex(const int32_t appIndex)
{
    std::lock_guard guard(wantLock_);
    want_.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);
}

int32_t AbilityRecord::GetWantAppIndex() const
{
    std::lock_guard guard(wantLock_);
    return want_.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0);
}

bool AbilityRecord::IsMinimizeFromUser() const
{
    return minimizeReason_;
}

void AbilityRecord::SetClearMissionFlag(bool clearMissionFlag)
{
    clearMissionFlag_= clearMissionFlag;
}

bool AbilityRecord::IsClearMissionFlag()
{
    return clearMissionFlag_;
}

int32_t AbilityRecord::GetMissionId() const
{
    return missionId_;
}

void AbilityRecord::SetSpecifiedFlag(const std::string &flag)
{
    specifiedFlag_ = flag;
}

std::string AbilityRecord::GetSpecifiedFlag() const
{
    return specifiedFlag_;
}

// new version  --start
bool AbilityRecord::IsStartedByCall() const
{
    return isStartedByCall_;
}

void AbilityRecord::SetStartedByCall(const bool isFlag)
{
    isStartedByCall_ = isFlag;
}

bool AbilityRecord::IsStartToBackground() const
{
    return isStartToBackground_;
}

void AbilityRecord::SetStartToBackground(const bool flag)
{
    isStartToBackground_ = flag;
}

bool AbilityRecord::IsStartToForeground() const
{
    return isStartToForeground_;
}

void AbilityRecord::SetStartToForeground(const bool flag)
{
    isStartToForeground_ = flag;
}

bool AbilityRecord::IsCallerSetProcess() const
{
    return isCallerSetProcess_.load();
}

void AbilityRecord::SetCallerSetProcess(const bool flag)
{
    isCallerSetProcess_.store(flag);
}

void AbilityRecord::PostStartAbilityByCallTimeoutTask(bool isHalf)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER(handler);

    auto timeoutTask = [isHalf, ability = shared_from_this()]() {
        std::string abilityName = ability->GetAbilityInfo().name;
        std::string bundleName = ability->GetAbilityInfo().bundleName;
        auto pid = ability->GetPid();
        auto uid = ability->GetUid();
        if (pid == 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "ability:%{public}s, app fork fail/not run", abilityName.c_str());
            return;
        }
        int typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
        std::string msgContent = "ability:" + abilityName + " ";
        std::string eventName;
        FreezeUtil::TimeoutState state = FreezeUtil::TimeoutState::BY_CALL;
        msgContent = isHalf ? msgContent + "call request half timeout." : msgContent + "call request timeout.";
        eventName = isHalf ? AppExecFwk::AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING
            : AppExecFwk::AppFreezeType::LIFECYCLE_TIMEOUT_WARNING;

        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s: uid: %{public}d, pid: %{public}d, bundleName: %{public}s, "
            "abilityName: %{public}s, msg: %{public}s", eventName.c_str(), uid, pid,
            bundleName.c_str(), abilityName.c_str(), msgContent.c_str());

        AppExecFwk::AppfreezeManager::ParamInfo info = { false, typeId, pid, eventName, bundleName };
        FreezeUtil::LifecycleFlow flow;
        if (ability->GetToken() != nullptr) {
            flow.token = ability->GetToken()->AsObject();
            flow.state = state;
        }
        info.msg = msgContent + "\nserver actions for ability:\n" +
            FreezeUtil::GetInstance().GetLifecycleEvent(flow.token)
            + "\nserver actions for app:\n" + FreezeUtil::GetInstance().GetAppLifecycleEvent(pid);
        if (!isHalf) {
            FreezeUtil::GetInstance().DeleteLifecycleEvent(flow.token);
            FreezeUtil::GetInstance().DeleteAppLifecycleEvent(pid);
        }
        AppExecFwk::AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info, flow);
    };
    auto timeoutMs = isHalf ? BY_CALL_HALF_TIMEOUT_MS : BY_CALL_TIMEOUT_MS;
    std::string taskPrefix = isHalf ? "by_call_half_timeout_" : "by_call_timeout_";
    handler->SubmitTask(timeoutTask, taskPrefix + std::to_string(recordId_), timeoutMs);
}

void AbilityRecord::CallRequest()
{
    CHECK_POINTER(scheduler_);
    // Async call request
    std::string entry = "AbilityRecord::CallRequest Begin";
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, entry);
    scheduler_->CallRequest();
    PostStartAbilityByCallTimeoutTask(true);
    PostStartAbilityByCallTimeoutTask(false);
}

void AbilityRecord::CancelStartAbilityByCallTimeoutTask() const
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail get AbilityEventHandler");
        return;
    }
    if (GetToken() && GetToken()->AsObject()) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(GetToken()->AsObject());
    }
    FreezeUtil::GetInstance().DeleteAppLifecycleEvent(GetPid());
    handler->CancelTask("by_call_timeout_" + std::to_string(recordId_));
    handler->CancelTask("by_call_half_timeout_" + std::to_string(recordId_));
}

bool AbilityRecord::CallRequestDone(const sptr<IRemoteObject> &callStub) const
{
    CHECK_POINTER_RETURN_BOOL(callContainer_);
    CancelStartAbilityByCallTimeoutTask();
    if (!callContainer_->CallRequestDone(callStub)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "call failed");
        return false;
    }
    return true;
}

ResolveResultType AbilityRecord::Resolve(const AbilityRequest &abilityRequest)
{
    auto callback = abilityRequest.connect;
    if (abilityRequest.callType != AbilityCallType::CALL_REQUEST_TYPE || !callback) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "only callType can create call record");
        return ResolveResultType::NG_INNER_ERROR;
    }
    if (!callContainer_) {
        callContainer_ = std::make_shared<CallContainer>();
        if (!callContainer_) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "mark_shared error");
            return ResolveResultType::NG_INNER_ERROR;
        }
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "create call record for this resolve. callerUid:%{public}d ,targetname:%{public}s",
        abilityRequest.callerUid,
        abilityRequest.abilityInfo.name.c_str());

    std::shared_ptr<CallRecord> callRecord = callContainer_->GetCallRecord(callback);
    if (!callRecord) {
        callRecord = CallRecord::CreateCallRecord(
            abilityRequest.callerUid, shared_from_this(), callback, abilityRequest.callerToken);
        if (!callRecord) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "mark_shared error");
            return ResolveResultType::NG_INNER_ERROR;
        }
    }

    callContainer_->AddCallRecord(callback, callRecord);

    if (callRecord->IsCallState(CallState::REQUESTED) && callRecord->GetCallStub()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "this record has requested.");
        if (!callRecord->SchedulerConnectDone()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "this callrecord has requested, but callback failed.");
            return ResolveResultType::NG_INNER_ERROR;
        }
        return ResolveResultType::OK_HAS_REMOTE_OBJ;
    }

    callRecord->SetCallState(CallState::REQUESTING);
    return ResolveResultType::OK_NO_REMOTE_OBJ;
}

bool AbilityRecord::ReleaseCall(const sptr<IAbilityConnection>& connect)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability release call record by callback.");
    CHECK_POINTER_RETURN_BOOL(callContainer_);

    return callContainer_->RemoveCallRecord(connect);
}

bool AbilityRecord::IsExistConnection(const sptr<IAbilityConnection> &connect)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability find call record by callback.");
    CHECK_POINTER_RETURN_BOOL(callContainer_);

    return callContainer_->IsExistConnection(connect);
}

bool AbilityRecord::IsNeedToCallRequest() const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability release call record by callback.");
    if (callContainer_ == nullptr) {
        return false;
    }

    return callContainer_->IsNeedToCallRequest();
}

void AbilityRecord::NotifyCallersOnTerminate()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Notify callers on terminate: %{public}s",
        abilityInfo_.name.c_str());

    if (callContainer_ == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "callContainer_ is null, no callers to notify");
        return;
    }

    AppExecFwk::ElementName element(abilityInfo_.deviceId, abilityInfo_.bundleName,
        abilityInfo_.name, abilityInfo_.moduleName);

    callContainer_->NotifyAllCallDisconnect(element);
}

void AbilityRecord::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ContinueAbility");
    CHECK_POINTER(lifecycleDeal_);

    lifecycleDeal_->ContinueAbility(deviceId, versionCode);
}

void AbilityRecord::SetSwitchingPause(bool state)
{
    isSwitchingPause_ = state;
}

bool AbilityRecord::IsSwitchingPause()
{
    return isSwitchingPause_;
}

void AbilityRecord::SetOwnerMissionUserId(int32_t userId)
{
    ownerMissionUserId_ = userId;
}

int32_t AbilityRecord::GetOwnerMissionUserId()
{
    return ownerMissionUserId_;
}

void AbilityRecord::DumpClientInfo(std::vector<std::string> &info, const std::vector<std::string> &params,
    bool isClient, bool dumpConfig) const
{
    if (!scheduler_ || !isReady_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "something nullptr.");
        return;
    }
    if (!isClient) {
        return;
    }
    std::unique_lock<ffrt::mutex> lock(dumpLock_);
    scheduler_->DumpAbilityInfo(params, info);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "dump begin wait");
    isDumpTimeout_ = false;
    std::chrono::milliseconds timeout { DUMP_TIMEOUT_MULTIPLE };
    if (dumpCondition_.wait_for(lock, timeout) == ffrt::cv_status::timeout) {
        isDumpTimeout_ = true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "dump done and begin parse");
    if (!isDumpTimeout_) {
        std::lock_guard<ffrt::mutex> infoLock(dumpInfoLock_);
        for (auto one : dumpInfos_) {
            info.emplace_back(one);
        }
    }

    if (!dumpConfig) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not dumpConfig");
        return;
    }
    AppExecFwk::Configuration config;
    if (DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config) == ERR_OK) {
        info.emplace_back("          configuration: " + config.GetName());
    }
}

void AbilityRecord::DumpAbilityInfoDone(std::vector<std::string> &infos)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    if (isDumpTimeout_) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, time out", __func__);
        return;
    }
    {
        std::lock_guard<ffrt::mutex> infoLock(dumpInfoLock_);
        dumpInfos_.clear();
        for (auto info : infos) {
            dumpInfos_.emplace_back(info);
        }
    }
    dumpCondition_.notify_all();
}

#ifdef SUPPORT_UPMS
void AbilityRecord::GrantUriPermission(Want &want, std::string targetBundleName, bool isSandboxApp,
    uint32_t tokenId, bool isNotifyCollaborator)
{
    // only for UIAbility and send result
    if (specifyTokenId_ > 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "specifyTokenId: %{public}u, cleaned", specifyTokenId_);
        tokenId = specifyTokenId_;
        specifyTokenId_ = 0;
    }
    // reject sandbox to grant uri permission by start ability
    auto caller = GetCallerRecord();
    if (caller && caller->appIndex_ > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sandbox can not grant UriPermission");
        return;
    }
    auto callerTokenId = tokenId > 0 ? tokenId :
        static_cast<uint32_t>(want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerTokenId:%{public}u, tokenId:%{public}u", callerTokenId, tokenId);
    GrantUriPermissionInfo grantInfo;
    grantInfo.callerTokenId = callerTokenId;
    grantInfo.collaboratorType = collaboratorType_;
    grantInfo.isSandboxApp = isSandboxApp;
    grantInfo.targetBundleName = targetBundleName;
    grantInfo.appIndex = appIndex_;
    grantInfo.userId = GetOwnerMissionUserId();
    grantInfo.flag = want.GetFlags();
    grantInfo.isNotifyCollaborator = isNotifyCollaborator;
    if (!isNotifyCollaborator) {
        grantInfo.targetAbilityName = abilityInfo_.name;
    }
    UriUtils::GetInstance().GrantUriPermission(want, grantInfo);
}

void AbilityRecord::GrantUriPermission(const std::vector<std::string> &uriVec, int32_t flag,
    const std::string &targetBundleName, uint32_t callerTokenId)
{
    if (uriVec.empty() || flag == 0 || targetBundleName.empty()) {
        return;
    }
    UriUtils::GetInstance().GrantUriPermission(uriVec, flag, targetBundleName, appIndex_, callerTokenId);
}

void AbilityRecord::GrantUriPermission()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    {
        std::lock_guard guard(wantLock_);
        GrantUriPermission(want_, abilityInfo_.applicationInfo.bundleName, false, 0, false);
    }
}
#endif // SUPPORT_UPMS

#ifdef WITH_DLP
void AbilityRecord::HandleDlpAttached()
{
    if (abilityInfo_.bundleName == DLP_BUNDLE_NAME) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->AddDlpManager(shared_from_this());
    }

    if (appIndex_ > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->AddDlpAbility(shared_from_this());
    }
}

void AbilityRecord::HandleDlpClosed()
{
    if (abilityInfo_.bundleName == DLP_BUNDLE_NAME) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->RemoveDlpManager(shared_from_this());
    }

    if (appIndex_ > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->RemoveDlpAbility(shared_from_this());
    }
}
#endif // WITH_DLP

void AbilityRecord::NotifyRemoveShellProcess(int32_t type)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "type is : %{public}d", type);
    if (abilityInfo_.bundleName == AppUtils::GetInstance().GetBrokerDelegateBundleName()) {
        auto collaborator = DelayedSingleton<AbilityManagerService>::GetInstance()->GetCollaborator(type);
        if (collaborator == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null collaborator");
            return;
        }
        int ret = collaborator->NotifyRemoveShellProcess(pid_, SHELL_ASSISTANT_DIETYPE, SHELL_ASSISTANT_DIEREASON);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify broker params pid: %{public}d", pid_);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, err: %{public}d", ret);
        }
    }
}

void AbilityRecord::NotifyMissionBindPid()
{
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    auto sessionInfo = GetSessionInfo();
    if (sessionInfo == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null sessionInfo");
        return;
    }
    int32_t persistentId = sessionInfo->persistentId;
    if (abilityInfo_.bundleName == AppUtils::GetInstance().GetBrokerDelegateBundleName()) {
        auto collaborator = DelayedSingleton<AbilityManagerService>::GetInstance()->GetCollaborator(
            CollaboratorType::RESERVE_TYPE);
        if (collaborator == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null collaborator");
            return;
        }
        collaborator->NotifyMissionBindPid(persistentId, pid_, GetOwnerMissionUserId());
    }
}

int32_t AbilityRecord::GetCurrentAccountId() const
{
    std::vector<int32_t> osActiveAccountIds;
    ErrCode ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
            QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "queryActiveOsAccountIds failed");
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "empty QueryActiveOsAccountIds");
        return DEFAULT_USER_ID;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "osActiveAccountId: %{public}d", osActiveAccountIds.front());
    return osActiveAccountIds.front();
}

void AbilityRecord::SetPendingState(AbilityState state)
{
    pendingState_.store(state);
}

AbilityState AbilityRecord::GetPendingState() const
{
    return pendingState_.load();
}

int32_t AbilityRecord::GetCollaboratorType() const
{
    return collaboratorType_;
}

std::string AbilityRecord::GetMissionAffinity() const
{
    return missionAffinity_;
}

void AbilityRecord::SetLockedState(bool lockedState)
{
    lockedState_ = lockedState;
}

bool AbilityRecord::GetLockedState()
{
    return lockedState_;
}

void AbilityRecord::SetAttachDebug(const bool isAttachDebug)
{
    isAttachDebug_ = isAttachDebug;
}

void AbilityRecord::SetAssertDebug(bool isAssertDebug)
{
    isAssertDebug_ = isAssertDebug;
}

void AbilityRecord::AddAbilityWindowStateMap(uint64_t uiExtensionComponentId,
    AbilityWindowState abilityWindowState)
{
    abilityWindowStateMap_[uiExtensionComponentId] = abilityWindowState;
}

void AbilityRecord::RemoveAbilityWindowStateMap(uint64_t uiExtensionComponentId)
{
    if (abilityWindowStateMap_.find(uiExtensionComponentId) != abilityWindowStateMap_.end()) {
        abilityWindowStateMap_.erase(uiExtensionComponentId);
    }
}

bool AbilityRecord::IsAbilityWindowReady()
{
    for (auto &item:abilityWindowStateMap_) {
        if (item.second == AbilityWindowState::BACKGROUNDING ||
            item.second == AbilityWindowState::TERMINATING) {
            return false;
        }
    }
    return true;
}

void AbilityRecord::SetAbilityWindowState(const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd, bool isFinished)
{
    if (sessionInfo == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null sessionInfo");
        return;
    }
    if (isFinished) {
        if (winCmd == WIN_CMD_FOREGROUND) {
            AddAbilityWindowStateMap(sessionInfo->uiExtensionComponentId, AbilityWindowState::FOREGROUND);
        } else if (winCmd == WIN_CMD_BACKGROUND) {
            AddAbilityWindowStateMap(sessionInfo->uiExtensionComponentId, AbilityWindowState::BACKGROUND);
        } else if (winCmd == WIN_CMD_DESTROY) {
            RemoveAbilityWindowStateMap(sessionInfo->uiExtensionComponentId);
        }
    } else {
        if (winCmd == WIN_CMD_FOREGROUND) {
            AddAbilityWindowStateMap(sessionInfo->uiExtensionComponentId, AbilityWindowState::FOREGROUNDING);
        } else if (winCmd == WIN_CMD_BACKGROUND) {
            AddAbilityWindowStateMap(sessionInfo->uiExtensionComponentId, AbilityWindowState::BACKGROUNDING);
        } else if (winCmd == WIN_CMD_DESTROY) {
            AddAbilityWindowStateMap(sessionInfo->uiExtensionComponentId, AbilityWindowState::TERMINATING);
        }
    }
}

int32_t AbilityRecord::CreateModalUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER_AND_RETURN(scheduler_, INNER_ERR);
    return scheduler_->CreateModalUIExtension(want);
}

void AbilityRecord::SetURI(const std::string &uri)
{
    uri_ = uri;
}

std::string AbilityRecord::GetURI() const
{
    if (uri_.empty()) {
        auto bundleName = abilityInfo_.bundleName;
        if (MultiInstanceUtils::IsMultiInstanceApp(abilityInfo_.applicationInfo)) {
            bundleName = bundleName + '-' + GetInstanceKey();
        } else if (AbilityRuntime::StartupUtil::IsSupportAppClone(abilityInfo_.extensionAbilityType)) {
            if (appIndex_ > 0) {
                bundleName = std::to_string(appIndex_) + bundleName;
            }
        }
        return AppExecFwk::ElementName(abilityInfo_.deviceId, bundleName,
            abilityInfo_.name, abilityInfo_.moduleName).GetURI();
    }
    return uri_;
}

void AbilityRecord::SetProcessName(const std::string &process)
{
    abilityInfo_.process = process;
}

std::string AbilityRecord::GetProcessName() const
{
    return abilityInfo_.process;
}

void AbilityRecord::SetCustomProcessFlag(const std::string &process)
{
    customProcessFlag_ = process;
}

std::string AbilityRecord::GetCustomProcessFlag() const
{
    return customProcessFlag_;
}

void AbilityRecord::SetExtensionProcessMode(const uint32_t &extensionProcessMode)
{
    extensionProcessMode_ = extensionProcessMode;
}

uint32_t AbilityRecord::GetExtensionProcessMode() const
{
    return extensionProcessMode_;
}

void AbilityRecord::SetUIExtensionAbilityId(const int32_t uiExtensionAbilityId)
{
    uiExtensionAbilityId_ = uiExtensionAbilityId;
}

int32_t AbilityRecord::GetUIExtensionAbilityId() const
{
    return uiExtensionAbilityId_;
}

bool AbilityRecord::BackgroundAbilityWindowDelayed()
{
    return backgroundAbilityWindowDelayed_.load();
}

void AbilityRecord::DoBackgroundAbilityWindowDelayed(bool needBackground)
{
    backgroundAbilityWindowDelayed_.store(needBackground);
}

bool AbilityRecord::IsSceneBoard() const
{
    return AbilityUtil::IsSceneBoard(abilityInfo_.bundleName, abilityInfo_.name);
}

void AbilityRecord::SetRestartAppFlag(bool isRestartApp)
{
    isRestartApp_ = isRestartApp;
}

bool AbilityRecord::GetRestartAppFlag() const
{
    return isRestartApp_;
}

void AbilityRecord::SetKillForPermissionUpdateFlag(bool isKillForPermissionUpdate)
{
    isKillForPermissionUpdate_ = isKillForPermissionUpdate;
}

bool AbilityRecord::GetKillForPermissionUpdateFlag() const
{
    return isKillForPermissionUpdate_;
}

void AbilityRecord::UpdateUIExtensionInfo(const WantParams &wantParams)
{
    if (!UIExtensionWrapper::IsUIExtension(GetAbilityInfo().extensionAbilityType)) {
        return;
    }

    std::lock_guard guard(wantLock_);
    if (want_.HasParameter(UIEXTENSION_ABILITY_ID)) {
        want_.RemoveParam(UIEXTENSION_ABILITY_ID);
    }
    want_.SetParam(UIEXTENSION_ABILITY_ID, wantParams.GetIntParam(UIEXTENSION_ABILITY_ID, -1));

    if (want_.HasParameter(UIEXTENSION_ROOT_HOST_PID)) {
        want_.RemoveParam(UIEXTENSION_ROOT_HOST_PID);
    }
    want_.SetParam(UIEXTENSION_ROOT_HOST_PID, wantParams.GetIntParam(UIEXTENSION_ROOT_HOST_PID, -1));
}

void AbilityRecord::SetSpecifyTokenId(uint32_t specifyTokenId)
{
    specifyTokenId_ = specifyTokenId;
}

void AbilityRecord::SetDebugAppByWaitingDebugFlag()
{
    if (!(abilityInfo_.applicationInfo.debug &&
        abilityInfo_.applicationInfo.appProvisionType == APP_PROVISION_TYPE_DEBUG) ||
        !system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Not meeting the set debugging conditions.");
        return;
    }

    if (IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->IsWaitingDebugApp(
        abilityInfo_.applicationInfo.bundleName))) {
        {
            std::lock_guard guard(wantLock_);
            want_.SetParam(AbilityConfig::DEBUG_APP, true);
        }
        launchDebugInfo_.isDebugAppSet = true;
        launchDebugInfo_.debugApp = true;
        IN_PROCESS_CALL_WITHOUT_RET(
            DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->ClearNonPersistWaitingDebugFlag());
    }
}

void AbilityRecord::SaveConnectWant(const Want &want)
{
    std::lock_guard guard(connectWantLock_);
    if (connectWant_ == nullptr) {
        connectWant_ = std::make_shared<Want>(want);
    }
}

void AbilityRecord::UpdateConnectWant()
{
    std::lock_guard guard(connectWantLock_);
    if (connectWant_ != nullptr) {
        SetWant(*connectWant_);
    }
}

void AbilityRecord::RemoveConnectWant()
{
    std::lock_guard guard(connectWantLock_);
    connectWant_.reset();
}

void AbilityRecord::UpdateDmsCallerInfo(Want &want)
{
    if (want.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0) != DMS_UID) {
        return;
    }
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, -1);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, -1);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, -1);

    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, want.GetStringParam(DMS_CALLER_BUNDLE_NAME));
    want.RemoveParam(DMS_CALLER_BUNDLE_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, want.GetStringParam(DMS_CALLER_ABILITY_NAME));
    want.RemoveParam(DMS_CALLER_ABILITY_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_NATIVE_NAME, want.GetStringParam(DMS_CALLER_NATIVE_NAME));
    want.RemoveParam(DMS_CALLER_NATIVE_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_APP_ID, want.GetStringParam(DMS_CALLER_APP_ID));
    want.RemoveParam(DMS_CALLER_APP_ID);
    want.SetParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER, want.GetStringParam(DMS_CALLER_APP_IDENTIFIER));
    want.RemoveParam(DMS_CALLER_APP_IDENTIFIER);
}

void AbilityRecord::SetDebugUIExtension()
{
    if (!UIExtensionWrapper::IsUIExtension(GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not UIExtension");
        return;
    }
    std::lock_guard guard(wantLock_);
    want_.SetParam(AbilityConfig::DEBUG_APP, true);
    launchDebugInfo_.isDebugAppSet = true;
    launchDebugInfo_.debugApp = true;
}

void AbilityRecord::NotifyAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
    CHECK_POINTER(lifecycleDeal_);
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_ERR_MSG, message },
    };
    lifecycleDeal_->NotifyAbilityRequestFailure(requestId, element, jsonObject.dump(), resultCode);
}

void AbilityRecord::NotifyAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element)
{
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->NotifyAbilityRequestSuccess(requestId, element);
}

void AbilityRecord::NotifyAbilitiesRequestDone(const std::string &requestKey, int32_t resultCode)
{
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->NotifyAbilitiesRequestDone(requestKey, resultCode);
}

void AbilityRecord::UpdateUIExtensionBindInfo(const WantParams &wantParams)
{
    if (!UIExtensionWrapper::IsUIExtension(GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "abilityType not match");
        return;
    }

    std::lock_guard guard(wantLock_);
    if (want_.HasParameter(UIEXTENSION_BIND_ABILITY_ID)) {
        want_.RemoveParam(UIEXTENSION_BIND_ABILITY_ID);
    }
    want_.SetParam(UIEXTENSION_BIND_ABILITY_ID, wantParams.GetIntParam(UIEXTENSION_BIND_ABILITY_ID, -1));

    if (want_.HasParameter(UIEXTENSION_NOTIFY_BIND)) {
        want_.RemoveParam(UIEXTENSION_NOTIFY_BIND);
    }
    want_.SetParam(UIEXTENSION_NOTIFY_BIND, wantParams.GetIntParam(UIEXTENSION_NOTIFY_BIND, -1));

    if (want_.HasParameter(UIEXTENSION_HOST_PID)) {
        want_.RemoveParam(UIEXTENSION_HOST_PID);
    }
    want_.SetParam(UIEXTENSION_HOST_PID, wantParams.GetIntParam(UIEXTENSION_HOST_PID, -1));

    if (want_.HasParameter(UIEXTENSION_HOST_UID)) {
        want_.RemoveParam(UIEXTENSION_HOST_UID);
    }
    want_.SetParam(UIEXTENSION_HOST_UID, wantParams.GetIntParam(UIEXTENSION_HOST_UID, -1));

    if (want_.HasParameter(UIEXTENSION_HOST_BUNDLENAME)) {
        want_.RemoveParam(UIEXTENSION_HOST_BUNDLENAME);
    }
    want_.SetParam(UIEXTENSION_HOST_BUNDLENAME, wantParams.GetStringParam(UIEXTENSION_HOST_BUNDLENAME));
}

void AbilityRecord::SendAppStartupTypeEvent(const AppExecFwk::AppStartType startType,
    const AppExecFwk::AppStartReason startReason)
{
    AAFwk::EventInfo eventInfo;
    auto abilityInfo = GetAbilityInfo();
    eventInfo.abilityName = abilityInfo.name;
    auto applicationInfo = GetApplicationInfo();
    eventInfo.bundleName = applicationInfo.name;
    eventInfo.versionName = applicationInfo.versionName;
    eventInfo.versionCode = applicationInfo.versionCode;
    eventInfo.pid = static_cast<int32_t>(GetPid());
    eventInfo.startType = static_cast<int32_t>(startType);
    eventInfo.startReason = static_cast<int32_t>(startReason);
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_STARTUP_TYPE, HISYSEVENT_BEHAVIOR, eventInfo);
}

void AbilityRecord::AddUIExtensionLaunchTimestamp()
{
    if (sessionInfo_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null sessionInfo_");
        return;
    }
    if (sessionInfo_->uiExtensionUsage != AAFwk::UIExtensionUsage::MODAL) {
        TAG_LOGD(AAFwkTag::UI_EXT, "not modal uiExtension");
        return;
    }
    std::lock_guard guard(wantLock_);
    struct timespec ts;
    int64_t launchTimestamp = -1;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        launchTimestamp = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
        int32_t high = static_cast<int32_t>(static_cast<uint32_t>(launchTimestamp >> 32));
        int32_t low = static_cast<int32_t>(static_cast<uint32_t>(launchTimestamp & 0xFFFFFFFFLL));
        want_.SetParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, high);
        want_.SetParam(UIEXTENSION_LAUNCH_TIMESTAMP_LOW, low);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "clock_gettime failed: %{public}s", strerror(errno));
    }
}

void AbilityRecord::RemoveUIExtensionLaunchTimestamp()
{
    std::lock_guard guard(wantLock_);
    want_.RemoveParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH);
    want_.RemoveParam(UIEXTENSION_LAUNCH_TIMESTAMP_LOW);
}

bool AbilityRecord::ReportAbilityConnectionRelations()
{
    auto recordCallerInfo = GetCallerInfo();
    CHECK_POINTER_RETURN_BOOL(recordCallerInfo);
    
    auto targetPid = GetPid();
    auto targetUid = GetUid();
    auto callerPid = recordCallerInfo->callerPid;
    auto callerUid = recordCallerInfo->callerUid;
    auto callerBundleName = recordCallerInfo->callerBundleName;
    auto targetBundleName = GetElementName().GetBundleName();
    if (targetPid <= 0 || targetUid <= 0 || callerPid <= 0 || callerUid <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid target process: targetPid=%{public}d, targetUid=%{public}d, "
            "callerPid=%{public}d, callerUid=%{public}d", targetPid, targetUid, callerPid, callerUid);
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "ConnectionInfo: targetPid=%{public}d, targetUid=%{public}d, targetBundleName=%{public}s, "
        "callerPid=%{public}d, callerUid=%{public}d, callerBundleName=%{public}s",
        targetPid, targetUid, targetBundleName.c_str(), callerPid, callerUid, callerBundleName.c_str());

    ForegroundAppConnectionInfo info(callerPid, targetPid, callerUid, targetUid, callerBundleName, targetBundleName);
    DelayedSingleton<ForegroundAppConnectionManager>::GetInstance()->AbilityAddPidConnection(info,
        GetAbilityRecordId());
    return true;
}

void AbilityRecord::ReportAbilityConnectionForResultTerminate()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call IsForResultTerminate");
    std::vector<int32_t> notifyCallerPids;
    int32_t curPid = GetPid();
    int32_t curRecordId = GetAbilityRecordId();
    {
        std::lock_guard guard(callerListLock_);
        for (auto caller : callerList_) {
            if (caller == nullptr) {
                continue;
            }
            auto requestCodeSet = caller->GetRequestCodeSet();
            bool isForResultTerminate = false;
            for (auto requestCode : requestCodeSet) {
                if (requestCode != DEFAULT_REQUEST_CODE) {
                    isForResultTerminate = true;
                    break;
                }
            }
            if (isForResultTerminate) {
                auto callerInfo = caller->GetCallerInfo();
                if (callerInfo == nullptr) {
                    TAG_LOGE(AAFwkTag::ABILITYMGR, "callerInfo is null");
                    continue;
                }
                TAG_LOGD(AAFwkTag::ABILITYMGR, "found forResult callerPid %{public}d", callerInfo->callerPid);
                notifyCallerPids.emplace_back(callerInfo->callerPid);
            }
        }
    }
    for (auto pid : notifyCallerPids) {
        DelayedSingleton<ForegroundAppConnectionManager>::GetInstance()->AbilityRemovePidConnection(pid,
            curPid, curRecordId);
    }
}

void AbilityRecord::SetPromotePriority(bool promotePriority)
{
    if (!promotePriority) {
        uiAbilityProperty_.reset();
        return;
    }
    if (uiAbilityProperty_ == nullptr) {
        uiAbilityProperty_ = std::make_shared<UIAbilityProperty>();
    }
    uiAbilityProperty_->promotePriority = promotePriority;
    uiAbilityProperty_->byCallCallerSaUid = IPCSkeleton::GetCallingUid();
    uiAbilityProperty_->byCallCallerSaPid = IPCSkeleton::GetCallingPid();
}

bool AbilityRecord::GetPromotePriority()
{
    return uiAbilityProperty_ != nullptr && uiAbilityProperty_->promotePriority && GetPid() > 0;
}

bool AbilityRecord::PromotePriority()
{
    if (IsStartedByCall() && GetPromotePriority()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "promoting priority: %{public}s", GetAbilityInfo().bundleName.c_str());
        ResSchedUtil::GetInstance().PromotePriorityToRSS(uiAbilityProperty_->byCallCallerSaUid,
            uiAbilityProperty_->byCallCallerSaPid, GetAbilityInfo().bundleName, GetUid(), GetPid());
        return true;
    }
    return false;
}

void AbilityRecord::AddPluginAbility(std::shared_ptr<AbilityRecord> pluginAbility)
{
    if (!pluginAbility) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "pluginAbility is null");
        return;
    }

    std::lock_guard<std::mutex> lock(pluginMutex_);

    // Check if already exists
    for (const auto weakPlugin : pluginAbilityList_) {
        auto plugin = weakPlugin.lock();
        if (plugin == pluginAbility) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Plugin %{public}s already exists in list",
                plugin->GetAbilityInfo().name.c_str());
            return;
        }
    }

    pluginAbilityList_.push_back(pluginAbility);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Add plugin: %{public}s to host: %{public}s",
        pluginAbility->GetAbilityInfo().name.c_str(), abilityInfo_.name.c_str());
}

void AbilityRecord::RemovePluginAbility(std::shared_ptr<AbilityRecord> pluginAbility)
{
    std::lock_guard<std::mutex> lock(pluginMutex_);

    for (auto it = pluginAbilityList_.begin(); it != pluginAbilityList_.end(); ++it) {
        auto plugin = it->lock();
        if (plugin == pluginAbility) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Remove plugin success");
            pluginAbilityList_.erase(it);
        }
    }
}

std::vector<std::shared_ptr<AbilityRecord>> AbilityRecord::GetPluginAbilities() const
{
    std::vector<std::shared_ptr<AbilityRecord>> validPlugins;
    std::lock_guard<std::mutex> lock(pluginMutex_);
    for (const auto weakPlugin : pluginAbilityList_) {
        auto plugin = weakPlugin.lock();
        if (plugin && plugin->GetAbilityState() != AbilityState::TERMINATING) {
            validPlugins.push_back(plugin);
        }
    }

    return validPlugins;
}

void AbilityRecord::ClearPluginAbilities()
{
    std::lock_guard<std::mutex> lock(pluginMutex_);
    pluginAbilityList_.clear();
}

void AbilityRecord::InitPluginAbility(const AbilityRequest &abilityRequest)
{
    if (!AbilityRuntime::StartupUtil::IsStartPlugin(abilityRequest.want)) {
        return;
    }

    isPluginAbility_ = true;

    auto hostAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (hostAbility == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Plugin %{public}s: cannot find host by callerToken",
            abilityInfo_.name.c_str());
        return;
    }

    hostAbility->AddPluginAbility(shared_from_this());

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Plugin initialized: host=%{public}s, plugin=%{public}s",
        hostAbility->GetAbilityInfo().name.c_str(), abilityInfo_.name.c_str());
}

void AbilityRecord::PluginCompleteTerminate()
{
    if (!IsPluginAbility()) {
        return;
    }
    auto hostAbility = GetCallerRecord();
    if (hostAbility) {
        hostAbility->RemovePluginAbility(shared_from_this());
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Plugin %{public}s removed from host %{public}s",
            GetAbilityInfo().name.c_str(),
            hostAbility->GetAbilityInfo().name.c_str());
    }
}
}  // namespace AAFwk
}  // namespace OHOS
