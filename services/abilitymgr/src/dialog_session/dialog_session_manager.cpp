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

#include "dialog_session_manager.h"

#include <random>
#include "ability_manager_service.h"
#include "ability_util.h"
#include "hitrace_meter.h"
#include "int_wrapper.h"
#include "modal_system_ui_extension.h"
#include "query_erms_manager.h"
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#include "session_manager_lite.h"
#endif
#include "session/host/include/zidl/session_interface.h"
#include "start_ability_utils.h"
#include "string_wrapper.h"
#include "want.h"

const std::string APP_LAUNCH_TRUSTLIST = "ohos.params.appLaunchTrustList";
namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::BundleInfo;
namespace {
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr int32_t ERMS_ISALLOW_RESULTCODE = 10;
constexpr const char* SUPPORT_CLOSE_ON_BLUR = "supportCloseOnBlur";
constexpr const char* DIALOG_SESSION_ID = "dialogSessionId";
constexpr const char* PICKER_ERMS_POLICY = "ability.params.picker.erms.policy";
constexpr const char* ATOMIC_SERVICE_SHORT_LINK = "atomicServiceShortLink";
}

DialogSessionManager &DialogSessionManager::GetInstance()
{
    static DialogSessionManager instance;
    return instance;
}

std::string DialogSessionManager::GenerateDialogSessionId()
{
    auto timestamp = std::chrono::system_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(timestamp).count();
    std::random_device seed;
    std::mt19937 rng(seed());
    std::uniform_int_distribution<int> uni(0, INT_MAX);
    int randomDigit = uni(rng);
    std::string dialogSessionId = std::to_string(time) + "_" + std::to_string(randomDigit);

    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto iter = dialogSessionInfoMap_.find(dialogSessionId);
    while (iter != dialogSessionInfoMap_.end()) {
        dialogSessionId += "_1";
        iter = dialogSessionInfoMap_.find(dialogSessionId);
    }
    return dialogSessionId;
}

void DialogSessionManager::SetStartupSessionInfo(const std::string &dialogSessionId,
    const AbilityRequest &abilityRequest)
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    std::shared_ptr<StartupSessionInfo> startupSessionInfo = std::make_shared<StartupSessionInfo>();
    startupSessionInfo->abilityRequest = abilityRequest;
    startupSessionInfoMap_[dialogSessionId] = startupSessionInfo;
}

void DialogSessionManager::SetDialogSessionInfo(const std::string &dialogSessionId,
    sptr<DialogSessionInfo> &dialogSessionInfo, std::shared_ptr<DialogCallerInfo> &dialogCallerInfo)
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    dialogSessionInfoMap_[dialogSessionId] = dialogSessionInfo;
    dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
}

void DialogSessionManager::SetQueryERMSInfo(const std::string &dialogSessionId,
    const AbilityRequest &abilityRequest)
{
    if (!abilityRequest.isQueryERMS) {
        return;
    }
    std::lock_guard<ffrt::mutex> guard(queryERMSInfoLock_);
    queryERMSInfoMap_.insert(std::pair<std::string, QueryERMSInfo>(dialogSessionId,
        {
            abilityRequest.callerTokenRecordId,
            abilityRequest.appId,
            abilityRequest.startTime,
            abilityRequest.isEmbeddedAllowed
        }));
}

bool DialogSessionManager::NotifyQueryERMSFinished(const std::string &dialogSessionId, bool isAllowed)
{
    QueryERMSInfo info;
    {
        std::lock_guard<ffrt::mutex> guard(queryERMSInfoLock_);
        auto it = queryERMSInfoMap_.find(dialogSessionId);
        if (it == queryERMSInfoMap_.end()) {
            return false;
        }
        TAG_LOGI(AAFwkTag::DIALOG, "found,id=%{public}s", dialogSessionId.c_str());
        info = it->second;
        queryERMSInfoMap_.erase(dialogSessionId);
    }
    AtomicServiceStartupRule rule = { isAllowed, info.isEmbeddedAllowed };
    QueryERMSManager::GetInstance().OnQueryFinished(info.recordId, info.appId, info.startTime, rule, ERR_OK);
    return true;
}

sptr<DialogSessionInfo> DialogSessionManager::GetDialogSessionInfo(const std::string &dialogSessionId) const
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto it = dialogSessionInfoMap_.find(dialogSessionId);
    if (it != dialogSessionInfoMap_.end()) {
        return it->second;
    }
    TAG_LOGI(AAFwkTag::DIALOG, "not find");
    return nullptr;
}

std::shared_ptr<DialogCallerInfo> DialogSessionManager::GetDialogCallerInfo(const std::string &dialogSessionId) const
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto it = dialogCallerInfoMap_.find(dialogSessionId);
    if (it != dialogCallerInfoMap_.end()) {
        return it->second;
    }
    TAG_LOGI(AAFwkTag::DIALOG, "not find");
    return nullptr;
}

std::shared_ptr<StartupSessionInfo> DialogSessionManager::GetStartupSessionInfo(
    const std::string &dialogSessionId) const
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto it = startupSessionInfoMap_.find(dialogSessionId);
    if (it != startupSessionInfoMap_.end()) {
        return it->second;
    }
    TAG_LOGI(AAFwkTag::DIALOG, "not find");
    return nullptr;
}

void DialogSessionManager::ClearDialogContext(const std::string &dialogSessionId)
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    dialogSessionInfoMap_.erase(dialogSessionId);
    dialogCallerInfoMap_.erase(dialogSessionId);
    startupSessionInfoMap_.erase(dialogSessionId);
    return;
}

void DialogSessionManager::ClearAllDialogContexts()
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    dialogSessionInfoMap_.clear();
    dialogCallerInfoMap_.clear();
    startupSessionInfoMap_.clear();
}

void DialogSessionManager::GenerateCallerAbilityInfo(AbilityRequest &abilityRequest,
    DialogAbilityInfo &callerAbilityInfo)
{
    sptr<IRemoteObject> callerToken = abilityRequest.callerToken;
    if (callerToken != nullptr) {
        auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
        CHECK_POINTER(callerRecord);
        callerAbilityInfo.bundleName = callerRecord->GetAbilityInfo().bundleName;
        callerAbilityInfo.moduleName = callerRecord->GetAbilityInfo().moduleName;
        callerAbilityInfo.abilityName = callerRecord->GetAbilityInfo().name;
        callerAbilityInfo.abilityIconId = callerRecord->GetAbilityInfo().iconId;
        callerAbilityInfo.abilityLabelId = callerRecord->GetAbilityInfo().labelId;
        callerAbilityInfo.bundleIconId = callerRecord->GetApplicationInfo().iconId;
        callerAbilityInfo.bundleLabelId = callerRecord->GetApplicationInfo().labelId;
        callerAbilityInfo.visible = callerRecord->GetAbilityInfo().visible;
        callerAbilityInfo.appIndex = callerRecord->GetApplicationInfo().appIndex;
        callerAbilityInfo.multiAppMode = callerRecord->GetApplicationInfo().multiAppMode;
        callerAbilityInfo.codePath = callerRecord->GetApplicationInfo().codePath;
        callerAbilityInfo.installSource = callerRecord->GetApplicationInfo().installSource;
    }
}

void DialogSessionManager::GenerateSelectorTargetAbilityInfos(std::vector<DialogAppInfo> &dialogAppInfos,
    std::vector<DialogAbilityInfo> &targetAbilityInfos)
{
    for (auto &dialogAppInfo : dialogAppInfos) {
        DialogAbilityInfo targetDialogAbilityInfo;
        targetDialogAbilityInfo.bundleName = dialogAppInfo.bundleName;
        targetDialogAbilityInfo.moduleName = dialogAppInfo.moduleName;
        targetDialogAbilityInfo.abilityName = dialogAppInfo.abilityName;
        targetDialogAbilityInfo.abilityIconId = dialogAppInfo.abilityIconId;
        targetDialogAbilityInfo.abilityLabelId = dialogAppInfo.abilityLabelId;
        targetDialogAbilityInfo.bundleIconId = dialogAppInfo.bundleIconId;
        targetDialogAbilityInfo.bundleLabelId = dialogAppInfo.bundleLabelId;
        targetDialogAbilityInfo.visible = dialogAppInfo.visible;
        targetDialogAbilityInfo.appIndex = dialogAppInfo.appIndex;
        targetDialogAbilityInfo.multiAppMode = dialogAppInfo.multiAppMode;
        targetDialogAbilityInfo.codePath = dialogAppInfo.codePath;
        targetDialogAbilityInfo.installSource = dialogAppInfo.installSource;
        targetAbilityInfos.emplace_back(targetDialogAbilityInfo);
    }
}

void DialogSessionManager::GenerateJumpTargetAbilityInfos(AbilityRequest &abilityRequest,
    std::vector<DialogAbilityInfo> &targetAbilityInfos)
{
    DialogAbilityInfo targetDialogAbilityInfo;
    targetDialogAbilityInfo.bundleName = abilityRequest.abilityInfo.bundleName;
    targetDialogAbilityInfo.moduleName = abilityRequest.abilityInfo.moduleName;
    targetDialogAbilityInfo.abilityName = abilityRequest.abilityInfo.name;
    targetDialogAbilityInfo.abilityIconId = abilityRequest.abilityInfo.iconId;
    targetDialogAbilityInfo.abilityLabelId = abilityRequest.abilityInfo.labelId;
    targetDialogAbilityInfo.bundleIconId = abilityRequest.abilityInfo.applicationInfo.iconId;
    targetDialogAbilityInfo.bundleLabelId = abilityRequest.abilityInfo.applicationInfo.labelId;
    targetDialogAbilityInfo.visible = abilityRequest.abilityInfo.visible;
    targetDialogAbilityInfo.appIndex = abilityRequest.abilityInfo.applicationInfo.appIndex;
    targetDialogAbilityInfo.multiAppMode = abilityRequest.abilityInfo.applicationInfo.multiAppMode;
    targetDialogAbilityInfo.codePath = abilityRequest.abilityInfo.applicationInfo.codePath;
    targetDialogAbilityInfo.installSource = abilityRequest.abilityInfo.applicationInfo.installSource;
    targetAbilityInfos.emplace_back(targetDialogAbilityInfo);
}

void DialogSessionManager::GenerateDialogCallerInfo(AbilityRequest &abilityRequest, int32_t userId,
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo, SelectorType type, bool needGrantUriPermission)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(dialogCallerInfo);
    dialogCallerInfo->type = type;
    dialogCallerInfo->callerToken = abilityRequest.callerToken;
    dialogCallerInfo->requestCode = abilityRequest.requestCode;
    dialogCallerInfo->targetWant = abilityRequest.want;
    dialogCallerInfo->userId = userId;
    dialogCallerInfo->needGrantUriPermission = needGrantUriPermission;
    dialogCallerInfo->callerAccessTokenId = abilityRequest.callerAccessTokenId;
}

void DialogSessionManager::NotifyAbilityRequestFailure(const std::string &dialogSessionId, const Want &want)
{
    auto callerInfo = GetDialogCallerInfo(dialogSessionId);
    CHECK_POINTER(callerInfo);
    auto requestId = callerInfo->targetWant.GetStringParam(KEY_REQUEST_ID);
    if (requestId.empty() || callerInfo->callerToken == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "no need to handle ability request");
        return;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerInfo->callerToken);
    CHECK_POINTER(abilityRecord);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "handling ability request failure, requestId=%{public}s", requestId.c_str());
    std::string message = "User refused redirection";
    if (callerInfo->type == SelectorType::IMPLICIT_START_SELECTOR) {
        message = "User closed the implicit startup picker";
    } else if (callerInfo->type == SelectorType::APP_CLONE_SELECTOR) {
        message = "User closed the app clone picker";
    } else if (callerInfo->type == SelectorType::INTERCEPTOR_SELECTOR) {
        message = "User closed the interceptor picker";
    }
    abilityRecord->NotifyAbilityRequestFailure(requestId, want.GetElement(), message);
}

int DialogSessionManager::SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllowed)
{
    if (NotifyQueryERMSFinished(dialogSessionId, isAllowed)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "query ERMS finished");
        ClearDialogContext(dialogSessionId);
        return ERR_OK;
    }
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = GetDialogCallerInfo(dialogSessionId);
    if (!isAllowed) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "user refuse to jump");
        HandleUserRejected(dialogSessionId, want, dialogCallerInfo);
        return ERR_OK;
    }
    std::shared_ptr<StartupSessionInfo> startupSessionInfo = GetStartupSessionInfo(dialogSessionId);
    if (startupSessionInfo != nullptr) {
        return NotifySCBToRecoveryAfterInterception(dialogSessionId, startupSessionInfo->abilityRequest);
    }
    if (dialogCallerInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dialogCallerInfo null");
        ClearDialogContext(dialogSessionId);
        return ERR_INVALID_VALUE;
    }
    auto targetWant = dialogCallerInfo->targetWant;
    if (dialogCallerInfo->type != SelectorType::INTERCEPTOR_SELECTOR) {
        targetWant.SetElement(want.GetElement());
    }
    targetWant.SetParam("isSelector", dialogCallerInfo->type != SelectorType::WITHOUT_SELECTOR);
    targetWant.SetParam(DIALOG_SESSION_ID, dialogSessionId);
    if (want.HasParameter(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY)) {
        int32_t appIndex = want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
        targetWant.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);
    }
    targetWant.RemoveParam(PICKER_ERMS_POLICY);
    if (want.HasParameter(PICKER_ERMS_POLICY)) {
        targetWant.SetParam(PICKER_ERMS_POLICY, want.GetParams().GetIntParam(PICKER_ERMS_POLICY, 1));
    }
    if (!targetWant.HasParameter(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY)) {
        targetWant.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    }
    sptr<IRemoteObject> callerToken = dialogCallerInfo->callerToken;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER_AND_RETURN(abilityMgr, INNER_ERR);
    int32_t ret = ERR_OK;
    int32_t atomicServiceShortLink = want.GetIntParam(ATOMIC_SERVICE_SHORT_LINK, 0);
    if (atomicServiceShortLink == 1) {
        ret = abilityMgr->OpenLink(targetWant, callerToken, dialogCallerInfo->userId,
            dialogCallerInfo->requestCode, false);
    } else {
        ret = abilityMgr->StartAbilityAsCallerDetails(targetWant, callerToken, callerToken, dialogCallerInfo->userId,
            dialogCallerInfo->requestCode, false, dialogCallerInfo->type == SelectorType::APP_CLONE_SELECTOR,
            dialogCallerInfo->callerAccessTokenId);
    }
    if (ret == ERR_OK) {
        ClearDialogContext(dialogSessionId);
        abilityMgr->RemoveSelectorIdentity(dialogCallerInfo->targetWant.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0));
    }
    return ret;
}

void DialogSessionManager::HandleUserRejected(const std::string &dialogSessionId, const Want &want,
    const std::shared_ptr<DialogCallerInfo> &dialogCallerInfo)
{
    NotifyAbilityRequestFailure(dialogSessionId, want);
    if (dialogCallerInfo != nullptr && dialogCallerInfo->sessionToken != nullptr) {
        auto session = iface_cast<Rosen::ISession>(dialogCallerInfo->sessionToken);
        if (session != nullptr) {
            sptr<SessionInfo> sessionInfo = sptr<SessionInfo>::MakeSptr();
            sessionInfo->errorCode = static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_INTERCEPTOR_REJECT);
            sessionInfo->errorReason = "userReject";
            Rosen::ExceptionInfo exceptionInfo;
            session->NotifySessionException(sessionInfo, exceptionInfo);
        }
    }
    ClearDialogContext(dialogSessionId);
}

int32_t DialogSessionManager::NotifySCBToRecoveryAfterInterception(const std::string &dialogSessionId,
    const AbilityRequest &abilityRequest)
{
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr is nullptr.");
        return INNER_ERR;
    }
    int ret = IN_PROCESS_CALL(abilityMgr->NotifySCBToRecoveryAfterInterception(abilityRequest));
    if (ret == ERR_OK) {
        ClearDialogContext(dialogSessionId);
    }
    return ret;
}

std::string DialogSessionManager::GenerateDialogSessionRecordCommon(AbilityRequest &abilityRequest, int32_t userId,
    const AAFwk::WantParams &parameters, std::vector<DialogAppInfo> &dialogAppInfos, SelectorType type,
    bool needGrantUriPermission)
{
    auto dialogSessionInfo = sptr<DialogSessionInfo>::MakeSptr();
    CHECK_POINTER_AND_RETURN(dialogSessionInfo, "");

    GenerateCallerAbilityInfo(abilityRequest, dialogSessionInfo->callerAbilityInfo);

    if (type != SelectorType::WITHOUT_SELECTOR) {
        GenerateSelectorTargetAbilityInfos(dialogAppInfos, dialogSessionInfo->targetAbilityInfos);
    } else {
        GenerateJumpTargetAbilityInfos(abilityRequest, dialogSessionInfo->targetAbilityInfos);
    }

    dialogSessionInfo->parameters = parameters;

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    GenerateDialogCallerInfo(abilityRequest, userId, dialogCallerInfo, type, needGrantUriPermission);

    std::string dialogSessionId = GenerateDialogSessionId();
    SetDialogSessionInfo(dialogSessionId, dialogSessionInfo, dialogCallerInfo);
    SetQueryERMSInfo(dialogSessionId, abilityRequest);

    return dialogSessionId;
}

int DialogSessionManager::CreateJumpModalDialog(AbilityRequest &abilityRequest, int32_t userId,
    const Want &replaceWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    AAFwk::WantParams parameters;

    parameters.SetParam("deviceType", AAFwk::String::Box(OHOS::system::GetDeviceType()));
    parameters.SetParam("userId", AAFwk::Integer::Box(userId));
    parameters.SetParam(ATOMIC_SERVICE_SHORT_LINK, AAFwk::Integer::Box(abilityRequest.atomicServiceShortLink));

    std::vector<DialogAppInfo> dialogAppInfos;
    std::string dialogSessionId = GenerateDialogSessionRecordCommon(abilityRequest, userId, parameters,
        dialogAppInfos, SelectorType::WITHOUT_SELECTOR);
    if (dialogSessionId == "") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generation failed");
        return ERR_INVALID_VALUE;
    }

    return CreateModalDialogCommon(replaceWant, abilityRequest.callerToken, dialogSessionId);
}

int DialogSessionManager::CreateImplicitSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want,
    int32_t userId, std::vector<DialogAppInfo> &dialogAppInfos, bool needGrantUriPermission)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    AAFwk::Want sessionWant;

    sessionWant.SetParam("deviceType", OHOS::system::GetDeviceType());
    sessionWant.SetParam("userId", userId);
    sessionWant.SetParam("action", abilityRequest.want.GetAction());
    sessionWant.SetParam("wantType", abilityRequest.want.GetType());
    sessionWant.SetParam("uri", abilityRequest.want.GetUriString());
    sessionWant.SetParam("entities", abilityRequest.want.GetEntities());
    sessionWant.SetParam("appselector.selectorType", static_cast<int>(SelectorType::IMPLICIT_START_SELECTOR));
    bool showCaller = abilityRequest.want.GetBoolParam("showCaller", false);
    sessionWant.SetParam("showCaller", showCaller);
    sessionWant.SetParam("ohos.ability.params.showDefaultPicker",
        abilityRequest.want.GetBoolParam("ohos.ability.params.showDefaultPicker", false));
    sessionWant.SetParam(ATOMIC_SERVICE_SHORT_LINK, abilityRequest.atomicServiceShortLink);
    if (abilityRequest.want.HasParameter(APP_LAUNCH_TRUSTLIST)) {
        sessionWant.SetParam(APP_LAUNCH_TRUSTLIST,
            abilityRequest.want.GetStringArrayParam(APP_LAUNCH_TRUSTLIST));
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ImplicitSelector get trustlist %{public}zu",
            sessionWant.GetStringArrayParam(APP_LAUNCH_TRUSTLIST).size());
        std::vector<std::string> receiveList = sessionWant.GetStringArrayParam(APP_LAUNCH_TRUSTLIST);
        for (const std::string& str : receiveList) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "in trustlist: %{public}s", str.c_str());
        }
    }
    if (abilityRequest.want.HasParameter("show_default_appselect")) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "show_default_appselect: %{public}d",
            abilityRequest.want.GetIntParam("show_default_appselect", -1));
        sessionWant.SetParam("show_default_appselect",
            abilityRequest.want.GetIntParam("show_default_appselect", -1));
    }

    std::string dialogSessionId = GenerateDialogSessionRecordCommon(abilityRequest, userId, sessionWant.GetParams(),
        dialogAppInfos, SelectorType::IMPLICIT_START_SELECTOR, needGrantUriPermission);
    if (dialogSessionId == "") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generation failed");
        return ERR_INVALID_VALUE;
    }

    return CreateModalDialogCommon(want, abilityRequest.callerToken, dialogSessionId);
}

int DialogSessionManager::CreateCloneSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want,
    int32_t userId, std::vector<DialogAppInfo> &dialogAppInfos, const std::string &replaceWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::WantParams parameters;

    parameters.SetParam("deviceType", AAFwk::String::Box(OHOS::system::GetDeviceType()));
    parameters.SetParam("userId", AAFwk::Integer::Box(userId));
    parameters.SetParam("appselector.selectorType",
        AAFwk::Integer::Box(static_cast<int>(SelectorType::APP_CLONE_SELECTOR)));
    if (replaceWant !=  "") {
        parameters.SetParam("ecological.replaceWant", AAFwk::String::Box(replaceWant));
    }
    parameters.SetParam(ATOMIC_SERVICE_SHORT_LINK, AAFwk::Integer::Box(abilityRequest.atomicServiceShortLink));

    std::string dialogSessionId = GenerateDialogSessionRecordCommon(abilityRequest, userId, parameters,
        dialogAppInfos, SelectorType::APP_CLONE_SELECTOR);
    if (dialogSessionId == "") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generation failed");
        return ERR_INVALID_VALUE;
    }

    return CreateModalDialogCommon(want, abilityRequest.callerToken, dialogSessionId);
}

int DialogSessionManager::CreateModalDialogCommon(const Want &replaceWant, sptr<IRemoteObject> callerToken,
    const std::string &dialogSessionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    (const_cast<Want &>(replaceWant)).SetParam(DIALOG_SESSION_ID, dialogSessionId);
    auto connection = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
    if (callerToken == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for system");
        (const_cast<Want &>(replaceWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
        (const_cast<Want &>(replaceWant)).SetParam(SUPPORT_CLOSE_ON_BLUR, true);
        return IN_PROCESS_CALL(connection->CreateModalUIExtension(replaceWant)) ? ERR_OK : INNER_ERR;
    }
    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    if (!callerRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerRecord null");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> token;
    bool isParent = false;
    int32_t ret = ERR_INVALID_VALUE;
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
        if (sceneSessionManager == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "sceneSessionManager null");
            return ERR_INVALID_VALUE;
        }
        ret = static_cast<int32_t>(IN_PROCESS_CALL(sceneSessionManager->IsFocusWindowParent(callerToken, isParent)));
        TAG_LOGD(AAFwkTag::ABILITYMGR, "isParent = %{public}d, ret = %{public}d", isParent, ret);
    } else {
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (!abilityMgr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr null");
            return INNER_ERR;
        }
        ret = IN_PROCESS_CALL(abilityMgr->GetTopAbility(token));
    }
#endif

    if (ret == ERR_OK && callerRecord->GetAbilityInfo().type == AppExecFwk::AbilityType::PAGE &&
        (isParent || token == callerToken)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for application");
        return callerRecord->CreateModalUIExtension(replaceWant);
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for system");
    (const_cast<Want &>(replaceWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
    (const_cast<Want &>(replaceWant)).SetParam(SUPPORT_CLOSE_ON_BLUR, true);
    return IN_PROCESS_CALL(connection->CreateModalUIExtension(replaceWant)) ? ERR_OK : INNER_ERR;
}

int DialogSessionManager::HandleErmsResult(AbilityRequest &abilityRequest, int32_t userId,
    const Want &replaceWant)
{
    std::string bundleName = abilityRequest.abilityInfo.bundleName;
    if (StartAbilityUtils::ermsResultCode < ERMS_ISALLOW_RESULTCODE ||
        !IsCreateCloneSelectorDialog(bundleName, userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "create jump modal dialog");
        return CreateJumpModalDialog(abilityRequest, userId, replaceWant);
    }
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr null");
        return INNER_ERR;
    }
    (const_cast<Want &>(replaceWant)).RemoveParam("ecological_experience_original_target");
    return abilityMgr->CreateCloneSelectorDialog(abilityRequest, userId, replaceWant.ToString());
}

int32_t DialogSessionManager::HandleErmsResultBySCB(AbilityRequest &abilityRequest, const Want &replaceWant)
{
    auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
    (const_cast<Want &>(replaceWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
    (const_cast<Want &>(replaceWant)).SetParam(SUPPORT_CLOSE_ON_BLUR, true);
    std::string dialogSessionId = GenerateDialogSessionId();
    if (dialogSessionId == "") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate dialog session record failed");
        return ERR_INVALID_VALUE;
    }
    SetStartupSessionInfo(dialogSessionId, abilityRequest);
    (const_cast<Want &>(replaceWant)).SetParam(DIALOG_SESSION_ID, dialogSessionId);
    return IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(replaceWant)) ?
        ERR_ECOLOGICAL_CONTROL_STATUS : INNER_ERR;
}

bool DialogSessionManager::IsCreateCloneSelectorDialog(const std::string &bundleName, int32_t userId)
{
    if (StartAbilityUtils::isWantWithAppCloneIndex) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "no clone index");
        StartAbilityUtils::isWantWithAppCloneIndex = false;
        return false;
    }
    auto appIndexes = StartAbilityUtils::GetCloneAppIndexes(bundleName, userId);
    if (appIndexes.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not create clone index");
        return false;
    }
    return true;
}

bool DialogSessionManager::UpdateExtensionWantWithDialogCallerInfo(AbilityRequest &abilityRequest,
    const sptr<IRemoteObject> &callerToken, bool isSCBCall)
{
    if (callerToken == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callerToken null");
        return false;
    }
    std::string dialogSessionId = abilityRequest.want.GetStringParam(DIALOG_SESSION_ID);
    if (dialogSessionId.empty()) {
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "dialogSessionId:%{public}s", dialogSessionId.c_str());
    auto dialogCallerInfo = GetDialogCallerInfo(dialogSessionId);
    if (dialogCallerInfo == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "dialogCallerInfo null");
        return false;
    }
    if (!dialogCallerInfo->needGrantUriPermission) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "no need grant uri permission");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "get dialog caller info");
    auto dialogTargetWant = dialogCallerInfo->targetWant;
    auto flag = dialogTargetWant.GetFlags();
    if ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not grant flag");
        return false;
    }
    if (!isSCBCall && (dialogCallerInfo->callerToken != callerToken)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not scb call or callerToken invalid");
        return false;
    }
    // only reserve grant uri flag
    abilityRequest.want.SetFlags(dialogTargetWant.GetFlags());
    abilityRequest.want.SetUri(dialogTargetWant.GetUri());
    auto uriVec = dialogTargetWant.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
    abilityRequest.want.SetParam(AbilityConfig::PARAMS_STREAM, uriVec);
    if (!isSCBCall) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not scb call");
        return true;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(dialogCallerInfo->callerToken);
    if (abilityRecord) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "set specifyTokenId");
        abilityRequest.specifyTokenId = abilityRecord->GetApplicationInfo().accessTokenId;
        return true;
    }
    return false;
}

void DialogSessionManager::OnlySetDialogCallerInfo(AbilityRequest &abilityRequest, int32_t userId,
    SelectorType type, const std::string &dialogSessionId, bool needGrantUriPermission)
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    GenerateDialogCallerInfo(abilityRequest, userId, dialogCallerInfo, type, needGrantUriPermission);
    if (abilityRequest.sessionInfo != nullptr) {
        dialogCallerInfo->sessionToken = abilityRequest.sessionInfo->sessionToken;
    }
    dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
}
}  // namespace AAFwk
}  // namespace OHOS
