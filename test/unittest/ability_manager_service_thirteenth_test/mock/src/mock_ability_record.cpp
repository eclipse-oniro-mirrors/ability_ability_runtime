/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "app_exit_reason_data_manager.h"
#include "app_utils.h"
#include "array_wrapper.h"
#include "accesstoken_kit.h"
#include "configuration_convertor.h"
#include "connection_state_manager.h"
#include "common_event_manager.h"
#include "error_msg_util.h"
#include "freeze_util.h"
#include "global_constant.h"
#include "hitrace_meter.h"
#include "image_source.h"
#include "keep_alive_process_manager.h"
#include "last_exit_detail_info.h"
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
#include "utils/state_utils.h"
#ifdef SUPPORT_GRAPHICS
#include "image_source.h"
#endif
#ifdef SUPPORT_SCREEN
#include "locale_config.h"
#endif

#include "mock_my_status.h"


namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AAFwk {
using namespace OHOS::Security;
using namespace OHOS::AAFwk::PermissionConstants;
using namespace OHOS::AbilityRuntime::GlobalConstant;
const std::string DEBUG_APP = "debugApp";
const std::string NATIVE_DEBUG = "nativeDebug";
const std::string PERF_CMD = "perfCmd";
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
constexpr const char* PARAM_SEND_RESULT_CALLER_BUNDLENAME = "ohos.anco.param.sendResultCallderBundleName";
constexpr const char* PARAM_SEND_RESULT_CALLER_TOKENID = "ohos.anco.param.sendResultCallerTokenId";
constexpr const char* PARAM_RESV_ANCO_IS_NEED_UPDATE_NAME = "ohos.anco.param.isNeedUpdateName";
constexpr const char* DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
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
const int VECTOR_SIZE = 2;
const int LOAD_TIMEOUT_ASANENABLED = 150;
const int TERMINATE_TIMEOUT_ASANENABLED = 150;
const int HALF_TIMEOUT = 2;
const int MAX_URI_COUNT = 500;
const int RESTART_SCENEBOARD_DELAY = 500;
constexpr int32_t DMS_UID = 5522;
constexpr int32_t SCHEDULER_DIED_TIMEOUT = 60000;

auto g_addLifecycleEventTask = [](sptr<Token> token, std::string &methodName) {
    CHECK_POINTER_LOG(token, "token is nullptr");
    std::string entry = std::string("AbilityRecord::") + methodName + "; the " + methodName + " lifecycle starts.";
    FreezeUtil::GetInstance().AddLifecycleEvent(token->AsObject(), entry);
};

Token::Token(std::weak_ptr<AbilityRecord> abilityRecord) : abilityRecord_(abilityRecord)
{}

Token::~Token()
{}

std::shared_ptr<AbilityRecord> Token::GetAbilityRecordByToken(sptr<IRemoteObject> token)
{
    return MyStatus::GetInstance().arGetAbilityRecord_;
}

std::shared_ptr<AbilityRecord> Token::GetAbilityRecord() const
{
    return MyStatus::GetInstance().arGetAbilityRecord_;
}

CallerRecord::CallerRecord(int requestCode, std::weak_ptr<AbilityRecord> caller)
    : requestCode_(requestCode), caller_(caller)
{
    auto callerAbilityRecord = caller.lock();
    if  (callerAbilityRecord != nullptr) {
        callerInfo_ = std::make_shared<CallerAbilityInfo>();
        callerInfo_->callerBundleName = callerAbilityRecord->GetAbilityInfo().bundleName;
        callerInfo_->callerAbilityName = callerAbilityRecord->GetAbilityInfo().name;
        callerInfo_->callerTokenId = callerAbilityRecord->GetApplicationInfo().accessTokenId;
        callerInfo_->callerUid =  callerAbilityRecord->GetUid();
        callerInfo_->callerPid =  callerAbilityRecord->GetPid();
        callerInfo_->callerAppCloneIndex = callerAbilityRecord->GetAppIndex();
    }
}

void LaunchDebugInfo::Update(const OHOS::AAFwk::Want &want)
{
    isDebugAppSet = want.HasParameter(DEBUG_APP);
    if (isDebugAppSet) {
        debugApp = want.GetBoolParam(DEBUG_APP, false);
    }
    isNativeDebugSet = want.HasParameter(NATIVE_DEBUG);
    if (isNativeDebugSet) {
        nativeDebug = want.GetBoolParam(NATIVE_DEBUG, false);
    }
    isPerfCmdSet = want.HasParameter(PERF_CMD);
    if (isPerfCmdSet) {
        perfCmd = want.GetStringParam(PERF_CMD);
    }
}

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
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    CHECK_POINTER_AND_RETURN(abilityRecord, nullptr);
    abilityRecord->SetUid(abilityRequest.uid);
    int32_t appIndex = 0;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(abilityRequest.want, appIndex);
    abilityRecord->SetAppIndex(appIndex);
    abilityRecord->SetSecurityFlag(abilityRequest.want.GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false));
    abilityRecord->SetCallerAccessTokenId(abilityRequest.callerAccessTokenId);
    abilityRecord->sessionInfo_ = abilityRequest.sessionInfo;
    if (AppUtils::GetInstance().IsMultiProcessModel() && abilityRequest.abilityInfo.isStageBasedModel &&
        abilityRequest.abilityInfo.type == AppExecFwk::AbilityType::PAGE &&
        !abilityRequest.customProcess.empty()) {
            abilityRecord->SetCustomProcessFlag(abilityRequest.customProcess);
        }
    if (abilityRequest.sessionInfo != nullptr) {
        abilityRecord->instanceKey_ = abilityRequest.sessionInfo->instanceKey;
    }
    abilityRecord->Init(abilityRequest);
    if (abilityRequest.startSetting != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityRequest.startSetting...");
        abilityRecord->SetStartSetting(abilityRequest.startSetting);
    }
    if (abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityRequest.callType is CALL_REQUEST_TYPE.");
        abilityRecord->SetStartedByCall(true);
    }
    if (AbilityRuntime::StartupUtil::IsStartPlugin(abilityRequest.want)) {
        abilityRecord->isPluginAbility_ = true;
    }
    abilityRecord->collaboratorType_ = abilityRequest.collaboratorType;
    abilityRecord->missionAffinity_ = abilityRequest.want.GetStringParam(PARAM_MISSION_AFFINITY_KEY);

    auto userId = abilityRequest.appInfo.uid / BASE_USER_RANGE;
    if ((userId == 0 ||
        AppUtils::GetInstance().InResidentWhiteList(abilityRequest.abilityInfo.bundleName)) &&
        DelayedSingleton<ResidentProcessManager>::GetInstance()->IsResidentAbility(
            abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name, userId)) {
        abilityRecord->keepAliveBundle_ = true;
    }

    return abilityRecord;
}

void AbilityRecord::Init(const AbilityRequest &)
{
    lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    token_ = new (std::nothrow) Token(weak_from_this());
    if (abilityInfo_.applicationInfo.isLauncherApp) {
        isLauncherAbility_ = true;
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
    SendEvent(AbilityManagerService::LOAD_HALF_TIMEOUT_MSG, loadTimeout / HALF_TIMEOUT);
    SendEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, loadTimeout);
    std::string methodName = "LoadAbility";
    g_addLifecycleEventTask(token_, methodName);
}

int AbilityRecord::LoadAbility(bool isShellCall, bool isStartupHide, pid_t callingPid,
    uint64_t callbackId, pid_t selfPid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadLifecycle: abilityName:%{public}s", abilityInfo_.name.c_str());
    startTime_ = AbilityUtil::SystemTimeMillis();
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
    want_.RemoveParam(Want::PARAM_APP_KEEP_ALIVE_ENABLED);
    if (KeepAliveProcessManager::GetInstance().IsKeepAliveBundle(abilityInfo_.applicationInfo.bundleName, -1)) {
        want_.SetParam(Want::PARAM_APP_KEEP_ALIVE_ENABLED, true);
        loadParam.isKeepAlive = true;
    }
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
        loadParam, abilityInfo_, abilityInfo_.applicationInfo, want_);
    want_.RemoveParam(IS_HOOK);
    want_.RemoveParam(ABILITY_OWNER_USERID);
    want_.RemoveParam(Want::PARAMS_REAL_CALLER_KEY);
    if (DelayedSingleton<AppScheduler>::GetInstance()->IsAttachDebug(abilityInfo_.bundleName)) {
        SetAttachDebug(true);
    }
    return result;
}

bool AbilityRecord::CanRestartRootLauncher()
{
    return true;
}

bool AbilityRecord::CanRestartResident()
{
    return true;
}

// only for UIAbility
void AbilityRecord::ForegroundAbility(uint32_t sceneFlag, bool hasLastWant)
{
}

void AbilityRecord::ForegroundUIExtensionAbility(uint32_t sceneFlag)
{
}

void AbilityRecord::ProcessForegroundAbility(uint32_t tokenId, const ForegroundOptions &options)
{
}

void AbilityRecord::PostForegroundTimeoutTask()
{
}

void AbilityRecord::RemoveForegroundTimeoutTask()
{
}

void AbilityRecord::RemoveLoadTimeoutTask()
{
}

#ifdef SUPPORT_SCREEN
void AbilityRecord::SetAbilityTransitionInfo(sptr<AbilityTransitionInfo>& info) const
{
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo()
{
    return nullptr;
}

std::shared_ptr<Want> AbilityRecord::GetWantFromMission() const
{
    return nullptr;
}

void AbilityRecord::SetShowWhenLocked(const AppExecFwk::AbilityInfo &abilityInfo,
    sptr<AbilityTransitionInfo> &info) const
{
}

void AbilityRecord::SetAbilityTransitionInfo(const AppExecFwk::AbilityInfo &abilityInfo,
    sptr<AbilityTransitionInfo> &info) const
{
}

sptr<IWindowManagerServiceHandler> AbilityRecord::GetWMSHandler() const
{
    return nullptr;
}

void AbilityRecord::SetWindowModeAndDisplayId(sptr<AbilityTransitionInfo> &info,
    const std::shared_ptr<Want> &want) const
{
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want) const
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    return info;
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(const AbilityRequest &abilityRequest) const
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    return info;
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want,
    const AbilityRequest &abilityRequest)
{
    return nullptr;
}

bool AbilityRecord::ReportAtomicServiceDrawnCompleteEvent()
{
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
}

bool AbilityRecord::PrepareTerminateAbility(bool isSCBCall)
{
    return false;
}

void AbilityRecord::PrepareTerminateAbilityDone(bool isTerminate)
{
}

void AbilityRecord::CancelPrepareTerminate()
{
}

int AbilityRecord::TerminateAbility()
{
    return 0;
}

const AppExecFwk::AbilityInfo &AbilityRecord::GetAbilityInfo() const
{
    return MyStatus::GetInstance().arGetAbilityInfo_;
}

const AppExecFwk::ApplicationInfo &AbilityRecord::GetApplicationInfo() const
{
    return MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo;
}

AbilityState AbilityRecord::GetAbilityState() const
{
    return currentState_;
}

WindowConfig AbilityRecord::GetAbilityWindowConfig() const
{
    return windowConfig_;
}

bool AbilityRecord::IsForeground() const
{
    return currentState_ == AbilityState::FOREGROUND || currentState_ == AbilityState::FOREGROUNDING;
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
}
#ifdef SUPPORT_SCREEN
void AbilityRecord::SetAbilityStateInner(AbilityState state)
{
}
#endif // SUPPORT_SCREEN
bool AbilityRecord::GetAbilityForegroundingFlag() const
{
    return isAbilityForegrounding_;
}

#ifdef SUPPORT_SCREEN
void AbilityRecord::SetAbilityState(AbilityState state)
{
}
#endif // SUPPORT_SCREEN
void AbilityRecord::SetScheduler(const sptr<IAbilityScheduler> &scheduler)
{
}

void AbilityRecord::AfterLoaded()
{
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
}

void AbilityRecord::Inactivate()
{
}

void AbilityRecord::Terminate(const Closure &task)
{
}

void AbilityRecord::ShareData(const int32_t &uniqueId)
{
}

void AbilityRecord::CommandAbility()
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "startId_:%{public}d.", startId_);
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
}

void AbilityRecord::SendResultByBackToCaller(const std::shared_ptr<AbilityResult> &result)
{
}

void AbilityRecord::SendSandboxSavefileResult(const Want &want, int resultCode, int requestCode)
{
}

void AbilityRecord::SendResultToCallers(bool schedulerdied)
{
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
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call, callingTokenId:%{public}u", callingTokenId);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        RecordSaCallerInfo(want);
    }
    if (!srcAbilityId.empty() && IsSystemAbilityCall(callerToken, callingTokenId)) {
        AddSystemAbilityCallerRecord(callerToken, requestCode, srcAbilityId);
        return;
    }
    CHECK_POINTER(abilityRecord);

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
}

void AbilityRecord::DumpAbilityState(
    std::vector<std::string> &info, bool isClient, const std::vector<std::string> &params)
{
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
}

void AbilityRecord::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
}

void AbilityRecord::OnProcessDied(bool isKeepAliveDied)
{
}

void AbilityRecord::NotifyAnimationAbilityDied()
{
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
    return (currentState_ == state);
}

bool AbilityRecord::IsActiveState() const
{
    return true;
}

void AbilityRecord::SendEvent(uint32_t msg, uint32_t timeOut, int32_t param, bool isExtension,
    const std::string &taskName)
{
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
        want_.SetParam(DEBUG_APP, launchDebugInfo_.debugApp);
    } else {
        want_.RemoveParam(DEBUG_APP);
    }
    if (launchDebugInfo_.isNativeDebugSet) {
        want_.SetParam(NATIVE_DEBUG, launchDebugInfo_.nativeDebug);
    } else {
        want_.RemoveParam(NATIVE_DEBUG);
    }
    if (launchDebugInfo_.isPerfCmdSet) {
        want_.SetParam(PERF_CMD, launchDebugInfo_.perfCmd);
    } else {
        want_.RemoveParam(PERF_CMD);
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
}

Want AbilityRecord::GetWant() const
{
    std::lock_guard guard(wantLock_);
    return want_;
}

void AbilityRecord::RemoveSignatureInfo()
{
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
    return false;
}

void AbilityRecord::SetWindowMode(int32_t windowMode)
{
    std::lock_guard guard(wantLock_);
    want_.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
}

void AbilityRecord::RemoveWindowMode()
{
}

void AbilityRecord::UpdateRecoveryInfo(bool hasRecoverInfo)
{
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
}

void AbilityRecord::SetRestarting(const bool isRestart, int32_t canRestartCount)
{
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
    return true;
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
    lastExitDetailInfo.timestamp = timestamp;
    lastExitDetailInfo.processName = processInfo.processName_;
    lastExitDetailInfo.exitMsg = killMsg;
    lifeCycleStateInfo_.launchParam.lastExitReason = CovertAppExitReasonToLastReason(exitReason.reason);
    lifeCycleStateInfo_.launchParam.lastExitMessage = exitMsg;
    lifeCycleStateInfo_.launchParam.lastExitDetailInfo = lastExitDetailInfo;
}

LastExitReason AbilityRecord::CovertAppExitReasonToLastReason(const Reason exitReason)
{
    return LASTEXITREASON_UNKNOWN;
}

void AbilityRecord::NotifyContinuationResult(int32_t result)
{
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

void AbilityRecord::CallRequest()
{
    scheduler_->CallRequest();
}

bool AbilityRecord::CallRequestDone(const sptr<IRemoteObject> &callStub) const
{
    return true;
}

ResolveResultType AbilityRecord::Resolve(const AbilityRequest &abilityRequest)
{
    return ResolveResultType::OK_NO_REMOTE_OBJ;
}

bool AbilityRecord::ReleaseCall(const sptr<IAbilityConnection>& connect)
{
    return callContainer_->RemoveCallRecord(connect);
}

bool AbilityRecord::IsExistConnection(const sptr<IAbilityConnection> &connect)
{
    return callContainer_->IsExistConnection(connect);
}

bool AbilityRecord::IsNeedToCallRequest() const
{
    return callContainer_->IsNeedToCallRequest();
}

void AbilityRecord::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
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
}

void AbilityRecord::DumpAbilityInfoDone(std::vector<std::string> &infos)
{
}

#ifdef SUPPORT_UPMS
void AbilityRecord::GrantUriPermission(Want &want, std::string targetBundleName, bool isSandboxApp, uint32_t tokenId)
{
}

void AbilityRecord::GrantUriPermission(const std::vector<std::string> &uriVec, int32_t flag,
    const std::string &targetBundleName, uint32_t callerTokenId)
{
}

void AbilityRecord::GrantUriPermission()
{
}
#endif // SUPPORT_UPMS

#ifdef WITH_DLP
void AbilityRecord::HandleDlpAttached()
{
}

void AbilityRecord::HandleDlpClosed()
{
}
#endif // WITH_DLP

void AbilityRecord::NotifyRemoveShellProcess(int32_t type)
{
}

void AbilityRecord::NotifyMissionBindPid()
{
}

int32_t AbilityRecord::GetCurrentAccountId() const
{
    return 0;
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
}

bool AbilityRecord::IsAbilityWindowReady()
{
    return true;
}

void AbilityRecord::SetAbilityWindowState(const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd, bool isFinished)
{
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
}

void AbilityRecord::SetSpecifyTokenId(uint32_t specifyTokenId)
{
    specifyTokenId_ = specifyTokenId;
}

void AbilityRecord::SetDebugAppByWaitingDebugFlag()
{
}

void AbilityRecord::SaveConnectWant(const Want &want)
{
    connectWant_ = std::make_shared<Want>(want);
}

void AbilityRecord::UpdateConnectWant()
{
}

void AbilityRecord::RemoveConnectWant()
{
}

void AbilityRecord::UpdateDmsCallerInfo(Want &want)
{
}

void AbilityRecord::SetDebugUIExtension()
{
}

void AbilityRecord::NotifyAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
}

void AbilityRecord::UpdateUIExtensionBindInfo(const WantParams &wantParams)
{
}
void AbilityRecord::SendTerminateAbilityErrorEvent(int32_t errCode)
{
}

void AbilityRecord::AddUIExtensionLaunchTimestamp()
{
}

void AbilityRecord::RemoveUIExtensionLaunchTimestamp()
{
}

bool AbilityRecord::ReportAbilityConnectionRelations()
{
    return true;
}

void AbilityRecord::SetPromotePriority(bool promotePriority)
{
}

bool AbilityRecord::GetPromotePriority()
{
    return false;
}

bool AbilityRecord::PromotePriority()
{
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
