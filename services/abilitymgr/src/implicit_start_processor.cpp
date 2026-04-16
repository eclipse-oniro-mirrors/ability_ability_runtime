/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "implicit_start_processor.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "app_utils.h"
#include "dialog_session_manager.h"
#include "ecological_rule/ability_ecological_rule_mgr_service.h"
#include "global_constant.h"
#include "hitrace_meter.h"
#include "start_ability_utils.h"
#include "startup_util.h"
#ifdef WITH_DLP
#include "dlp_file_kits.h"
#endif // WITH_DLP
#ifdef APP_DOMAIN_VERIFY_ENABLED
#include "app_domain_verify_mgr_client.h"
#endif // APP_DOMAIN_VERIFY_ENABLED
#ifdef SUPPORT_SCREEN
#include "utils/ability_permission_util.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AAFwk {
const size_t IDENTITY_LIST_MAX_SIZE = 10;
const size_t TRUSTLIST_MAX_SIZE = 50;

const std::string BLACK_ACTION_SELECT_DATA = "ohos.want.action.select";
const std::string ACTION_VIEW = "ohos.want.action.viewData";
const std::string STR_PHONE = "phone";
const std::string STR_DEFAULT = "default";
const std::string TYPE_ONLY_MATCH_WILDCARD = "reserved/wildcard";
const std::string SHOW_DEFAULT_PICKER_FLAG = "ohos.ability.params.showDefaultPicker";
const std::string PARAM_ABILITY_APPINFOS = "ohos.ability.params.appInfos";
const std::string ANCO_PENDING_REQUEST = "ancoPendingRequest";
const int NFC_CALLER_UID = 1027;
const int NFC_QUERY_LENGTH = 2;
const std::string OPEN_LINK_APP_LINKING_ONLY = "appLinkingOnly";
const std::string HTTP_SCHEME_NAME = "http";
const std::string HTTPS_SCHEME_NAME = "https";
const std::string FILE_SCHEME_NAME = "file";
const std::string APP_CLONE_INDEX = "ohos.extra.param.key.appCloneIndex";
const std::string APP_LAUNCH_TRUSTLIST = "ohos.params.appLaunchTrustList";
constexpr const char* SUPPORT_ACTION_START_SELECTOR = "persist.sys.ability.support.action_start_selector";

void SendAbilityEvent(const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    ffrt::submit([eventName, type, eventInfo]() {
        EventReport::SendAbilityEvent(eventName, type, eventInfo);
        }, ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));
}

bool ImplicitStartProcessor::IsExtensionInWhiteList(AppExecFwk::ExtensionAbilityType type)
{
    switch (type) {
        case AppExecFwk::ExtensionAbilityType::FORM: return true;
        case AppExecFwk::ExtensionAbilityType::INPUTMETHOD: return true;
        case AppExecFwk::ExtensionAbilityType::WALLPAPER: return true;
        case AppExecFwk::ExtensionAbilityType::WINDOW: return true;
        case AppExecFwk::ExtensionAbilityType::THUMBNAIL: return true;
        case AppExecFwk::ExtensionAbilityType::PREVIEW: return true;
        default: return false;
    }
}

bool ImplicitStartProcessor::IsImplicitStartAction(const Want &want)
{
    auto element = want.GetElement();
    if (!element.GetAbilityName().empty()) {
        return false;
    }

    if (want.GetIntParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE) != ScreenMode::IDLE_SCREEN_MODE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not use implicit startup process");
        return false;
    }

    if (want.GetAction() != BLACK_ACTION_SELECT_DATA &&
        !want.HasParameter(AppExecFwk::INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG)) {//intent openlink do not Implicit compare
        TAG_LOGI(AAFwkTag::ABILITYMGR, "implicit start, action:%{public}s", want.GetAction().data());
        return true;
    }

    return false;
}

int ImplicitStartProcessor::CheckImplicitCallPermission(const AbilityRequest& abilityRequest)
{
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER_AND_RETURN(abilityMgr, ERR_INVALID_VALUE);
    bool isBackgroundCall = true;
    if (abilityMgr->IsCallFromBackground(abilityRequest, isBackgroundCall, false) != ERR_OK) {
        return ERR_CHECK_CALL_FROM_BACKGROUND_FAILED;
    }
    if (!isBackgroundCall) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "hap not background");
        return ERR_OK;
    }
    auto ret = AAFwk::PermissionVerification::GetInstance()->VerifyBackgroundCallPermission(isBackgroundCall);
    if (ret) {
        return ERR_OK;
    }
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "permission failed");
        return CHECK_PERMISSION_FAILED;
    }
    auto sessionInfo = callerAbility->GetSessionInfo();
    int32_t persistentId = (sessionInfo == nullptr) ? -1 : sessionInfo->persistentId;
    TAG_LOGE(AAFwkTag::ABILITYMGR, "permission failed %{public}s,%{public}d",
        callerAbility->GetAbilityInfo().name.c_str(), persistentId);
    return CHECK_PERMISSION_FAILED;
}

int ImplicitStartProcessor::ImplicitStartAbility(AbilityRequest &request, int32_t userId, int32_t windowMode,
    const std::string &replaceWantString, bool isAppCloneSelector)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "implicit start ability by type: %{public}d", request.callType);
    bool isNfcCalling = (IPCSkeleton::GetCallingUid() == NFC_CALLER_UID);
    auto sysDialogScheduler = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    CHECK_POINTER_AND_RETURN(sysDialogScheduler, ERR_NULL_SYS_DIALOG_SCHEDULER);

    auto result = CheckImplicitCallPermission(request);
    if (ERR_OK != result) {
        return result;
    }
    std::vector<DialogAppInfo> dialogAppInfos;
    request.want.RemoveParam(APP_CLONE_INDEX);
    GenerateRequestParam genReqParam;
    int32_t ret = ERR_OK;
    if (isAppCloneSelector) {
        ret = GenerateAbilityRequestByAppIndexes(userId, request, dialogAppInfos);
    } else {
        ret = GenerateAbilityRequestByAction(userId, request, dialogAppInfos, genReqParam);
    }
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate request failed");
        return ret;
    }
    AbilityUtil::WantSetParameterWindowMode(request.want, windowMode);
    request.callerAccessTokenId = IPCSkeleton::GetCallingTokenID();

    auto identity = IPCSkeleton::ResetCallingIdentity();
    auto startAbilityTask = [imp = shared_from_this(), request, userId, identity]
        (const std::string& bundle, const std::string& abilityName, int32_t appIndex) mutable {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "callback");

        // reset calling indentity
        IPCSkeleton::SetCallingIdentity(identity);

        AAFwk::Want targetWant = request.want;
        targetWant.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);
        targetWant.SetElementName(bundle, abilityName);
        return imp->CallStartAbilityInner(userId, targetWant, request, request.callType);
    };

    int32_t tokenId = request.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, request.callerAccessTokenId);
    AddIdentity(tokenId, identity);
    // reset calling indentity
    IPCSkeleton::SetCallingIdentity(identity);

    if (!isAppCloneSelector && request.want.HasParameter(APP_LAUNCH_TRUSTLIST)) {
        TrustlistIntersectionProcess(request, dialogAppInfos, userId);
    }

    if (dialogAppInfos.size() == 0 &&
        (request.want.GetFlags() & Want::FLAG_START_WITHOUT_TIPS) == Want::FLAG_START_WITHOUT_TIPS) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "hint dialog generate fail");
        return ERR_IMPLICIT_START_ABILITY_FAIL;
    }

    AAFwk::Want want;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (dialogAppInfos.size() == 0 && AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        ret = sysDialogScheduler->GetSelectorDialogWant(dialogAppInfos, request.want, want, request.callerToken);
        if (ret != ERR_OK && ret != ERR_APP_SELECTOR_NOT_EXISTS) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSelectorDialogWant failed");
            return ret;
        }
        if (want.GetBoolParam("isCreateAppGallerySelector", false)) {
            want.RemoveParam("isCreateAppGallerySelector");
            bool needGrantUriPermission = true;
            TAG_LOGD(AAFwkTag::ABILITYMGR, "openLinkOptions hideFailureTipDialog: %{public}d",
                request.hideFailureTipDialog);
            if (!request.hideFailureTipDialog) {
                DialogSessionManager::GetInstance().CreateImplicitSelectorModalDialog(request, want, userId,
                    dialogAppInfos, needGrantUriPermission);
            }
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, show tips dialog");
        Want dialogWant = sysDialogScheduler->GetTipsDialogWant(request.callerToken);
        IN_PROCESS_CALL_WITHOUT_RET(abilityMgr->StartAbility(dialogWant));
        return ERR_IMPLICIT_START_ABILITY_FAIL;
    } else if (dialogAppInfos.size() == 0 && !AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        std::string type = MatchTypeAndUri(request.want);
        ret = sysDialogScheduler->GetPcSelectorDialogWant(dialogAppInfos, request.want, want, type,
            userId, request.callerToken);
        if (ret != ERR_OK && ret != ERR_APP_SELECTOR_NOT_EXISTS) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetPcSelectorDialogWant failed");
            return ret;
        }
        if (want.GetBoolParam("isCreateAppGallerySelector", false)) {
            want.RemoveParam("isCreateAppGallerySelector");
            if (!request.hideFailureTipDialog) {
                DialogSessionManager::GetInstance().CreateImplicitSelectorModalDialog(request, want, userId,
                    dialogAppInfos);
            }
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        std::vector<DialogAppInfo> dialogAllAppInfos;
        genReqParam.isMoreHapList = true;
        ret = IN_PROCESS_CALL(GenerateAbilityRequestByAction(userId, request, dialogAllAppInfos, genReqParam));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "request failed");
            return ret;
        }
        if (dialogAllAppInfos.size() == 0) {
            Want dialogWant = IN_PROCESS_CALL(sysDialogScheduler->GetTipsDialogWant(request.callerToken));
            IN_PROCESS_CALL_WITHOUT_RET(abilityMgr->StartAbility(dialogWant));
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        ret = sysDialogScheduler->GetPcSelectorDialogWant(dialogAllAppInfos, request.want, want,
            TYPE_ONLY_MATCH_WILDCARD, userId, request.callerToken);
        if (ret != ERR_OK && ret != ERR_APP_SELECTOR_NOT_EXISTS) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetPcSelectorDialogWant failed");
            return ret;
        }
        return IN_PROCESS_CALL(abilityMgr->StartAbility(request.want, request.callerToken));
    }

    //There is a default opening method add Only one application supports
    bool defaultPicker = false;
    defaultPicker = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, defaultPicker);
    if (isNfcCalling && dialogAppInfos.size() == 1 && !dialogAppInfos[0].isAppLink) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ImplicitQueryInfos, is nfc calling.");
        defaultPicker = true;
    }
    if (dialogAppInfos.size() == 1 && !defaultPicker) {
        auto info = dialogAppInfos.front();
        // Compatible with the action's sunset scene
        if (!IsActionImplicitStart(request.want, genReqParam.findDefaultApp)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ImplicitQueryInfos success,target ability: %{public}s",
                info.abilityName.data());
            return IN_PROCESS_CALL(startAbilityTask(info.bundleName, info.abilityName, info.appIndex));
        }
    }

    if (AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ImplicitQueryInfos success, multiple apps available");
        ret = sysDialogScheduler->GetSelectorDialogWant(dialogAppInfos, request.want, want, request.callerToken);
        if (ret != ERR_OK && ret != ERR_APP_SELECTOR_NOT_EXISTS) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSelectorDialogWant failed");
            return ret;
        }
        if (want.GetBoolParam("isCreateAppGallerySelector", false)) {
            want.RemoveParam("isCreateAppGallerySelector");
            if (isAppCloneSelector) {
                return DialogSessionManager::GetInstance().CreateCloneSelectorModalDialog(request, want,
                    userId, dialogAppInfos, replaceWantString);
            }
            bool needGrantUriPermission = true;
            return DialogSessionManager::GetInstance().CreateImplicitSelectorModalDialog(request,
                want, userId, dialogAppInfos, needGrantUriPermission);
        }
        return IN_PROCESS_CALL(abilityMgr->ImplicitStartAbilityAsCaller(request.want, request.callerToken, nullptr));
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ImplicitQueryInfos success, multiple apps available in pc");
    std::string type = MatchTypeAndUri(request.want);

    ret = sysDialogScheduler->GetPcSelectorDialogWant(dialogAppInfos, request.want, want,
        type, userId, request.callerToken);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetPcSelectorDialogWant failed");
        return INNER_ERR;
    }
    if (want.GetBoolParam("isCreateAppGallerySelector", false)) {
        want.RemoveParam("isCreateAppGallerySelector");
        if (isAppCloneSelector) {
            return DialogSessionManager::GetInstance().CreateCloneSelectorModalDialog(request, want, userId,
                dialogAppInfos, replaceWantString);
        }
        return DialogSessionManager::GetInstance().CreateImplicitSelectorModalDialog(request, want, userId,
            dialogAppInfos);
    }
    return IN_PROCESS_CALL(abilityMgr->ImplicitStartAbilityAsCaller(request.want, request.callerToken, nullptr));
}

std::string ImplicitStartProcessor::MatchTypeAndUri(const AAFwk::Want &want)
{
    std::string type = want.GetType();
    if (type.empty()) {
        auto uri = want.GetUriString();
        auto suffixIndex = uri.rfind('.');
        if (suffixIndex == std::string::npos) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, uri:%{public}s-%{private}s",
                want.GetUri().GetScheme().c_str(), uri.c_str());
            return "";
        }
        type = uri.substr(suffixIndex);
#ifdef WITH_DLP
        if (Security::DlpPermission::DlpFileKits::IsDlpFileBySuffix(type)) {
            auto suffixDlpIndex = uri.rfind('.', suffixIndex - 1);
            if (suffixDlpIndex == std::string::npos) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, uri: %{public}s-%{private}s",
                    want.GetUri().GetScheme().c_str(), uri.c_str());
                return "";
            }
            type = uri.substr(suffixDlpIndex, suffixIndex - suffixDlpIndex);
        }
#endif // WITH_DLP
    }
    return type;
}

static void ProcessLinkType(std::vector<AppExecFwk::AbilityInfo> &abilityInfos)
{
    bool appLinkingExist = false;
    bool defaultAppExist = false;
    if (!abilityInfos.size()) {
        return;
    }
    for (const auto &info : abilityInfos) {
        if (info.linkType == AppExecFwk::LinkType::APP_LINK) {
            appLinkingExist = true;
        }
        if (info.linkType == AppExecFwk::LinkType::DEFAULT_APP) {
            defaultAppExist = true;
        }
    }
    if (!appLinkingExist && !defaultAppExist) {
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "appLinkingExist: %{public}d, defaultAppExist: %{public}d",
        appLinkingExist, defaultAppExist);
    for (auto it = abilityInfos.begin(); it != abilityInfos.end();) {
        if (it->linkType == AppExecFwk::LinkType::APP_LINK) {
            it++;
            continue;
        }
        if (it->linkType == AppExecFwk::LinkType::DEFAULT_APP && appLinkingExist) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s default deleted.", it->name.c_str());
            it = abilityInfos.erase(it);
            continue;
        }
        if (it->linkType == AppExecFwk::LinkType::DEEP_LINK) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s deleted.", it->name.c_str());
            it = abilityInfos.erase(it);
            continue;
        }
        it++;
    }
}

void ImplicitStartProcessor::OnlyKeepReserveApp(std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos, const AbilityRequest &request)
{
    if (!request.uriReservedFlag) {
        return;
    }
    if (extensionInfos.size() > 0) {
        extensionInfos.clear();
    }

    for (auto it = abilityInfos.begin(); it != abilityInfos.end();) {
        if (it->bundleName == request.reservedBundleName) {
            it++;
            continue;
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "reserve App %{public}s dismatch with bundleName %{public}s",
                request.reservedBundleName.c_str(), it->bundleName.c_str());
            it = abilityInfos.erase(it);
        }
    }
}

#ifdef APP_DOMAIN_VERIFY_ENABLED
static bool IsApplinkExist(const std::vector<AppExecFwk::AbilityInfo> &abilityInfos)
{
    if (!abilityInfos.size()) {
        return false;
    }
    for (const auto &info : abilityInfos) {
        if (info.linkType == AppExecFwk::LinkType::APP_LINK) {
            return true;
        }
    }
    return false;
}

bool ImplicitStartProcessor::NeedQueryFromAG(const AbilityRequest &request, bool applinkExist)
{
    bool isOpenLink = request.want.HasParameter(OPEN_LINK_APP_LINKING_ONLY);
    if (!isOpenLink) {
        return false;
    }
    bool appLinkingOnly = request.want.GetBoolParam(OPEN_LINK_APP_LINKING_ONLY, false);
    if (appLinkingOnly) {
        return false;
    }
    std::string linkUriScheme = request.want.GetUri().GetScheme();
    if (linkUriScheme != HTTPS_SCHEME_NAME && linkUriScheme != HTTP_SCHEME_NAME) {
        return false;
    }
    if (applinkExist) {
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "need query from ag");
    return true;
}

int ImplicitStartProcessor::ImplicitStartAG(int32_t userId, AbilityRequest &request,
    std::vector<DialogAppInfo> &dialogAppInfos, GenerateRequestParam &genReqParam, bool &queryAGSuccess)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    std::string linkUri = request.want.GetUri().ToString();
    Want agWant;
    auto ret = 0;
    ret = OHOS::AppDomainVerify::AppDomainVerifyMgrClient::GetInstance()->QueryAppDetailsWant(linkUri, agWant);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryAppDetailsWant failed, ret:%{public}d", ret);
        return ret;
    }
    queryAGSuccess = true;
    request.want = agWant;
    genReqParam.fromImplicitStartAG = true;
    return GenerateAbilityRequestByAction(userId, request, dialogAppInfos, genReqParam);
}
#endif // APP_DOMAIN_VERIFY_ENABLED

int ImplicitStartProcessor::GenerateAbilityRequestByAction(int32_t userId, AbilityRequest &request,
    std::vector<DialogAppInfo> &dialogAppInfos, GenerateRequestParam &genReqParam)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s.", __func__);
    // get abilityinfos from bms
    auto bundleMgrHelper = GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, GET_ABILITY_SERVICE_FAILED);
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL_URI
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool withDefault = false;
    withDefault = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, withDefault) ? false : true;
    bool appLinkingOnly = false;
    bool isOpenLink = false;
    isOpenLink = request.want.HasParameter(OPEN_LINK_APP_LINKING_ONLY);
    appLinkingOnly = request.want.GetBoolParam(OPEN_LINK_APP_LINKING_ONLY, false);

    if (IPCSkeleton::GetCallingUid() == NFC_CALLER_UID &&
        !request.want.GetStringArrayParam(PARAM_ABILITY_APPINFOS).empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "caller source: NFC");
        QueryBmsAppInfos(request, userId, dialogAppInfos);
    }

    if (!StartAbilityUtils::IsCallFromAncoShellOrBroker(request.callerToken)) {
        request.want.RemoveParam(ANCO_PENDING_REQUEST);
    }

    if (appLinkingOnly) {
        abilityInfoFlag = static_cast<uint32_t>(abilityInfoFlag) |
            static_cast<uint32_t>(AppExecFwk::GetAbilityInfoFlag::GET_ABILITY_INFO_WITH_APP_LINKING);
    }

    if (request.uriReservedFlag) {
        abilityInfoFlag = static_cast<uint32_t>(abilityInfoFlag) |
            static_cast<uint32_t>(AppExecFwk::GetAbilityInfoFlag::GET_ABILITY_INFO_ONLY_SYSTEM_APP);
    }

    if (isOpenLink) {
        std::string linkUriScheme = request.want.GetUri().GetScheme();
        if (linkUriScheme == HTTPS_SCHEME_NAME || linkUriScheme == HTTP_SCHEME_NAME) {
            request.want.SetAction(ACTION_VIEW);
        }
    }

    IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->ImplicitQueryInfos(
        request.want, abilityInfoFlag, userId, withDefault, abilityInfos, extensionInfos,
        genReqParam.findDefaultApp));

    OnlyKeepReserveApp(abilityInfos, extensionInfos, request);
    if (isOpenLink && extensionInfos.size() > 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "clear extensionInfos");
        extensionInfos.clear();
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "ImplicitQueryInfos, abilityInfo size : %{public}zu, extensionInfos size: %{public}zu", abilityInfos.size(),
        extensionInfos.size());

    KioskManager::GetInstance().FilterAbilityInfos(abilityInfos);
    if (appLinkingOnly && abilityInfos.size() == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not match app");
        return ERR_IMPLICIT_START_ABILITY_FAIL;
    }

#ifdef APP_DOMAIN_VERIFY_ENABLED
    ErrCode result = ERR_OK;
    bool applinkExist = IsApplinkExist(abilityInfos);
    bool queryAGSuccess = false;
    if (!genReqParam.fromImplicitStartAG && NeedQueryFromAG(request, applinkExist)) {
        result = static_cast<int>(ImplicitStartAG(userId, request, dialogAppInfos, genReqParam, queryAGSuccess));
        if (queryAGSuccess) {
            return result;
        }
    }
#endif // APP_DOMAIN_VERIFY_ENABLED

    if (!appLinkingOnly) {
        ProcessLinkType(abilityInfos);
    }

    if (auto ret = StartAbilityUtils::HandleSelfRedirection(request.isFromOpenLink, abilityInfos); ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "disallow self redirection");
        return ret;
    }

#ifdef WITH_DLP
    if (request.want.GetBoolParam(AbilityUtil::DLP_PARAMS_SANDBOX, false)) {
        Security::DlpPermission::DlpFileKits::ConvertAbilityInfoWithSupportDlp(request.want, abilityInfos);
        extensionInfos.clear();
    }
#endif // WITH_DLP

    if (abilityInfos.size() + extensionInfos.size() > 1) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "filter applications by erms");
        bool ret = FilterAbilityList(request.want, abilityInfos, extensionInfos, userId);
        FindAppClone(abilityInfos, extensionInfos, genReqParam.isAppCloneSelector);
        if (!ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "FilterAbilityList failed");
        }
    }

    auto isExtension = request.callType == AbilityCallType::START_EXTENSION_TYPE;

    Want implicitwant;
    std::string typeName = MatchTypeAndUri(request.want);

    implicitwant.SetAction(request.want.GetAction());
    implicitwant.SetType(TYPE_ONLY_MATCH_WILDCARD);
    std::vector<AppExecFwk::AbilityInfo> implicitAbilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> implicitExtensionInfos;
    std::vector<std::string> infoNames;
    if (!AppUtils::GetInstance().IsSelectorDialogDefaultPossion() && genReqParam.isMoreHapList) {
        IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->ImplicitQueryInfos(implicitwant, abilityInfoFlag, userId,
            withDefault, implicitAbilityInfos, implicitExtensionInfos, genReqParam.findDefaultApp));
        if (implicitAbilityInfos.size() != 0 && typeName != TYPE_ONLY_MATCH_WILDCARD) {
            for (auto implicitAbilityInfo : implicitAbilityInfos) {
                infoNames.emplace_back(implicitAbilityInfo.bundleName + "#" +
                    implicitAbilityInfo.moduleName + "#" + implicitAbilityInfo.name);
            }
        }
    }

    if (abilityInfos.size() == 1) {
        auto skillUri =  abilityInfos.front().skillUri;
        SetTargetLinkInfo(skillUri, request.want);
        if (abilityInfos.front().linkType == AppExecFwk::LinkType::APP_LINK) {
            EventInfo eventInfo;
            eventInfo.bundleName = abilityInfos.front().bundleName;
            eventInfo.callerBundleName = request.want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
            eventInfo.uri = request.want.GetUriString();
            auto pos = eventInfo.uri.find("?");
            if (pos != std::string::npos) {
                eventInfo.uri = eventInfo.uri.substr(0, pos);
            }
            SendAbilityEvent(EventName::START_ABILITY_BY_APP_LINKING, HISYSEVENT_BEHAVIOR, eventInfo);
        }
    }

    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "for (const auto &info : abilityInfos)");
        bool isExistDefaultApp = IsExistDefaultApp(userId, typeName);
        for (const auto &info : abilityInfos) {
            AddInfoParam param = {
                .isExtension = isExtension,
                .isMoreHapList = genReqParam.isMoreHapList,
                .withDefault = withDefault,
                .isExistDefaultApp = isExistDefaultApp,
                .userId = userId,
                .typeName = typeName,
                .infoNames = infoNames,
                .info = info
            };
            AddAbilityInfoToDialogInfos(param, dialogAppInfos);
        }
    }

    for (const auto &info : extensionInfos) {
        if (!isExtension || !CheckImplicitStartExtensionIsValid(request, info)) {
            continue;
        }
        DialogAppInfo dialogAppInfo;
        dialogAppInfo.abilityName = info.name;
        dialogAppInfo.bundleName = info.bundleName;
        dialogAppInfo.abilityIconId = info.iconId;
        dialogAppInfo.abilityLabelId = info.labelId;
        dialogAppInfo.bundleIconId = info.applicationInfo.iconId;
        dialogAppInfo.bundleLabelId = info.applicationInfo.labelId;
        dialogAppInfo.visible = info.visible;
        dialogAppInfo.appIndex = info.applicationInfo.appIndex;
        dialogAppInfo.multiAppMode = info.applicationInfo.multiAppMode;
        dialogAppInfo.codePath = info.applicationInfo.codePath;
        dialogAppInfo.installSource = info.applicationInfo.installSource;
        dialogAppInfos.emplace_back(dialogAppInfo);
    }
    KioskManager::GetInstance().FilterDialogAppInfos(dialogAppInfos);

    return ERR_OK;
}

int ImplicitStartProcessor::GenerateAbilityRequestByAppIndexes(int32_t userId, AbilityRequest &request,
    std::vector<DialogAppInfo> &dialogAppInfos)
{
    auto appIndexes = StartAbilityUtils::GetCloneAppIndexes(request.want.GetBundle(), userId);
    if (appIndexes.size() > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "too large appIndexes");
        return ERR_TOO_LARGE_APPINDEXES;
    }
    auto bms = GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    auto abilityInfoFlag = static_cast<uint32_t>(AbilityRuntime::StartupUtil::BuildAbilityInfoFlag()) |
        static_cast<uint32_t>(AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL);
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    abilityInfos.emplace_back(request.abilityInfo);
    for (auto &appIndex: appIndexes) {
        AppExecFwk::AbilityInfo abilityInfo;
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "abilityName: %{public}s, appIndex: %{public}d, userId: %{public}d",
            request.want.GetElement().GetAbilityName().c_str(), appIndex, userId);
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryCloneAbilityInfo(request.want.GetElement(), abilityInfoFlag, appIndex,
            abilityInfo, userId));
        if (abilityInfo.name.empty() || abilityInfo.bundleName.empty()) {
            int32_t ret = FindExtensionInfo(request.want, abilityInfoFlag, userId, appIndex, abilityInfo);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "query info failed");
                return ret;
            }
        }
        abilityInfos.emplace_back(abilityInfo);
    }
    for (const auto &info : abilityInfos) {
        DialogAppInfo dialogAppInfo;
        dialogAppInfo.abilityName = info.name;
        dialogAppInfo.bundleName = info.bundleName;
        dialogAppInfo.moduleName = info.moduleName;
        dialogAppInfo.abilityIconId = info.iconId;
        dialogAppInfo.abilityLabelId = info.labelId;
        dialogAppInfo.bundleIconId = info.applicationInfo.iconId;
        dialogAppInfo.bundleLabelId = info.applicationInfo.labelId;
        dialogAppInfo.visible = info.visible;
        dialogAppInfo.appIndex = info.applicationInfo.appIndex;
        dialogAppInfo.multiAppMode = info.applicationInfo.multiAppMode;
        dialogAppInfo.codePath = info.applicationInfo.codePath;
        dialogAppInfo.installSource = info.applicationInfo.installSource;
        dialogAppInfos.emplace_back(dialogAppInfo);
    }
    return ERR_OK;
}

int ImplicitStartProcessor::FindExtensionInfo(const Want &want, int32_t flags, int32_t userId,
    int32_t appIndex, AppExecFwk::AbilityInfo &abilityInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bms = GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "abilityName: %{public}s, appIndex: %{public}d, userId: %{public}d",
        want.GetElement().GetAbilityName().c_str(), appIndex, userId);
    IN_PROCESS_CALL_WITHOUT_RET(bms->QueryCloneExtensionAbilityInfoWithAppIndex(want.GetElement(),
        flags, appIndex, extensionInfo, userId));
    if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionInfo empty.");
        return RESOLVE_ABILITY_ERR;
    }
    if (AbilityRuntime::StartupUtil::IsSupportAppClone(extensionInfo.type)) {
        AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo);
        return ERR_OK;
    }
    return ERR_APP_CLONE_INDEX_INVALID;
}

int ImplicitStartProcessor::QueryBmsAppInfos(AbilityRequest &request, int32_t userId,
    std::vector<DialogAppInfo> &dialogAppInfos)
{
    auto bundleMgrHelper = GetBundleManagerHelper();
    std::vector<AppExecFwk::AbilityInfo> bmsApps;
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL_URI
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION;
    std::vector<std::string> apps = request.want.GetStringArrayParam(PARAM_ABILITY_APPINFOS);
    for (std::string appInfoStr : apps) {
        AppExecFwk::AbilityInfo abilityInfo;
        std::vector<std::string> appInfos = ImplicitStartProcessor::SplitStr(appInfoStr, '/');
        if (appInfos.empty() || appInfos.size() != NFC_QUERY_LENGTH) {
            continue;
        }
        std::string bundleName = appInfos[0];
        std::string abilityName = appInfos[1];
        std::string queryAbilityName = bundleName.append(abilityName);
        Want want;
        want.SetElementName(appInfos[0], queryAbilityName);

        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "abilityName: %{public}s, userId: %{public}d", want.GetElement().GetAbilityName().c_str(), userId);
        IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(want, abilityInfoFlag,
            userId, abilityInfo));
        if (!abilityInfo.name.empty() && !abilityInfo.bundleName.empty() && !abilityInfo.moduleName.empty()) {
            bmsApps.emplace_back(abilityInfo);
        }
    }
    if (!bmsApps.empty()) {
        for (const auto &abilityInfo : bmsApps) {
            DialogAppInfo dialogAppInfo;
            dialogAppInfo.abilityName = abilityInfo.name;
            dialogAppInfo.bundleName = abilityInfo.bundleName;
            dialogAppInfo.moduleName = abilityInfo.moduleName;
            dialogAppInfo.abilityIconId = abilityInfo.iconId;
            dialogAppInfo.abilityLabelId = abilityInfo.labelId;
            dialogAppInfo.bundleIconId = abilityInfo.applicationInfo.iconId;
            dialogAppInfo.bundleLabelId = abilityInfo.applicationInfo.labelId;
            dialogAppInfo.visible = abilityInfo.visible;
            dialogAppInfo.appIndex = abilityInfo.applicationInfo.appIndex;
            dialogAppInfo.multiAppMode = abilityInfo.applicationInfo.multiAppMode;
            dialogAppInfo.codePath = abilityInfo.applicationInfo.codePath;
            dialogAppInfo.installSource = abilityInfo.applicationInfo.installSource;
            dialogAppInfos.emplace_back(dialogAppInfo);
        }
    }
    return ERR_OK;
}

std::vector<std::string> ImplicitStartProcessor::SplitStr(const std::string& str, char delimiter)
{
    std::stringstream ss(str);
    std::vector<std::string> result;
    std::string s;
    while (std::getline(ss, s, delimiter)) {
        result.push_back(s);
    }
    return result;
}

bool ImplicitStartProcessor::CheckImplicitStartExtensionIsValid(const AbilityRequest &request,
    const AppExecFwk::ExtensionAbilityInfo &extensionInfo)
{
    if (!request.want.GetElement().GetBundleName().empty()) {
        return true;
    }
    TAG_LOGD(
        AAFwkTag::ABILITYMGR, "ImplicitStartExtension type: %{public}d.", static_cast<int32_t>(extensionInfo.type));
    if (!IsExtensionInWhiteList(extensionInfo.type)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ImplicitStart not allowed");
        return false;
    }
    return true;
}

int32_t ImplicitStartProcessor::ImplicitStartAbilityInner(const Want &targetWant,
    const AbilityRequest &request, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER_AND_RETURN(abilityMgr, ERR_INVALID_VALUE);

    int32_t result = ERR_OK;
    switch (request.callType) {
        case AbilityCallType::START_OPTIONS_TYPE: {
            StartOptions startOptions = request.startOptions;
            auto displayId = targetWant.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, 0);
            auto windowMode = targetWant.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, 0);
            startOptions.SetDisplayID(static_cast<int32_t>(displayId));
            startOptions.SetWindowMode(static_cast<int32_t>(windowMode));
            startOptions.requestId_ = targetWant.GetStringParam(KEY_REQUEST_ID);
            result = abilityMgr->ImplicitStartAbility(
                targetWant, startOptions, request.callerToken, userId, request.requestCode);
            break;
        }
        case AbilityCallType::START_SETTINGS_TYPE: {
            CHECK_POINTER_AND_RETURN(request.startSetting, ERR_INVALID_VALUE);
            result = abilityMgr->ImplicitStartAbility(
                targetWant, *request.startSetting, request.callerToken, userId, request.requestCode);
            break;
        }
        case AbilityCallType::START_EXTENSION_TYPE:
            result = abilityMgr->ImplicitStartExtensionAbility(
                targetWant, request.callerToken, userId, request.extensionType);
            break;
        default:
            StartAbilityWrapParam startAbilityWrapParam = {
                targetWant, request.callerToken, request.requestCode, false, userId, false, 0, false, true };
            result = abilityMgr->StartAbilityWrap(startAbilityWrapParam);
            break;
    }

    return result;
}

int ImplicitStartProcessor::CallStartAbilityInner(int32_t userId,
    const Want &want, const AbilityRequest &request, const AbilityCallType &callType)
{
    EventInfo eventInfo;
    eventInfo.userId = userId;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();

    if (callType == AbilityCallType::INVALID_TYPE) {
        eventInfo.calleeId = static_cast<int32_t>(CalleeId::IMPLICIT_START_PROCESSOR_CALL_START_ABILITY_INNER);
        SendAbilityEvent(EventName::START_ABILITY, HISYSEVENT_BEHAVIOR, eventInfo);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ability:%{public}s, bundle:%{public}s", eventInfo.abilityName.c_str(),
        eventInfo.bundleName.c_str());

    auto ret = ImplicitStartAbilityInner(want, request, userId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CallStartAbilityInner failed: %{public}d", ret);
        if (callType == AbilityCallType::INVALID_TYPE) {
            eventInfo.errCode = ERR_DO_CLOSURE_CALLBACK_FAILED;
            SendAbilityEvent(EventName::START_ABILITY_ERROR, HISYSEVENT_FAULT, eventInfo);
        }
    }
    return ret;
}

std::shared_ptr<AppExecFwk::BundleMgrHelper> ImplicitStartProcessor::GetBundleManagerHelper()
{
    if (iBundleManagerHelper_ == nullptr) {
        iBundleManagerHelper_ = AbilityUtil::GetBundleManagerHelper();
    }
    return iBundleManagerHelper_;
}

sptr<AppExecFwk::IDefaultApp> ImplicitStartProcessor::GetDefaultAppProxy()
{
    auto bundleMgrHelper = GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundleMgrHelper");
        return nullptr;
    }
    auto defaultAppProxy = bundleMgrHelper->GetDefaultAppProxy();
    if (defaultAppProxy == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null defaultAppProxy");
        return nullptr;
    }
    return defaultAppProxy;
}

bool ImplicitStartProcessor::FilterAbilityList(const Want &want, std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos, int32_t userId)
{
    ErmsCallerInfo callerInfo;
    GetEcologicalCallerInfo(want, callerInfo, userId);
    int ret = IN_PROCESS_CALL(AbilityEcologicalRuleMgrServiceClient::GetInstance()->
        EvaluateResolveInfos(want, callerInfo, 0, abilityInfos, extensionInfos));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resolve infos failed");
        return false;
    }
    return true;
}

void ImplicitStartProcessor::GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId)
{
    callerInfo.packageName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    callerInfo.uid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    callerInfo.pid = want.GetIntParam(Want::PARAM_RESV_CALLER_PID, IPCSkeleton::GetCallingPid());
    callerInfo.targetAppType = ErmsCallerInfo::TYPE_INVALID;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_INVALID;

    auto bundleMgrHelper = GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrHelper empty");
        return;
    }

    std::string targetBundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo targetAppInfo;
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "targetBundleName: %{public}s, userId: %{public}d", targetBundleName.c_str(), userId);
    bool getTargetResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(targetBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    if (!getTargetResult) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get targetAppInfo failed");
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the target type  is atomic service");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the target type is app");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the target type is invalid type");
    }

    std::string callerBundleName;
    ErrCode err = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerInfo.uid, callerBundleName));
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerBundleName empty");
        return;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(callerBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    if (!getCallerResult) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "get callerAppInfo failed");
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the caller type  is atomic service");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the caller type is app");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the caller type is invalid type");
    }
}

void ImplicitStartProcessor::AddIdentity(int32_t tokenId, std::string identity)
{
    std::lock_guard guard(identityListLock_);
    if (identityList_.size() == IDENTITY_LIST_MAX_SIZE) {
        identityList_.pop_front();
        identityList_.emplace_back(IdentityNode(tokenId, identity));
        return;
    }
    identityList_.emplace_back(IdentityNode(tokenId, identity));
}

void ImplicitStartProcessor::ResetCallingIdentityAsCaller(int32_t tokenId, bool flag)
{
    std::lock_guard guard(identityListLock_);
    for (auto it = identityList_.begin(); it != identityList_.end(); it++) {
        if (it->tokenId == tokenId) {
            IPCSkeleton::SetCallingIdentity(it->identity);
            if (flag) {
                identityList_.erase(it);
            }
            return;
        }
    }
}

void ImplicitStartProcessor::RemoveIdentity(int32_t tokenId)
{
    std::lock_guard guard(identityListLock_);
    for (auto it = identityList_.begin(); it != identityList_.end(); it++) {
        if (it->tokenId == tokenId) {
            identityList_.erase(it);
            return;
        }
    }
}

void ImplicitStartProcessor::AddAbilityInfoToDialogInfos(const AddInfoParam &param,
    std::vector<DialogAppInfo> &dialogAppInfos)
{
    if (param.isExtension && param.info.type != AbilityType::EXTENSION) {
        return;
    }
    if (!AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        bool isDefaultFlag = param.withDefault && param.isExistDefaultApp;
        if (!param.isMoreHapList && !isDefaultFlag &&
            std::find(param.infoNames.begin(), param.infoNames.end(),
            (param.info.bundleName + "#" + param.info.moduleName + "#" + param.info.name)) != param.infoNames.end()) {
            return;
        }
    }
    DialogAppInfo dialogAppInfo;
    dialogAppInfo.abilityName = param.info.name;
    dialogAppInfo.bundleName = param.info.bundleName;
    dialogAppInfo.moduleName = param.info.moduleName;
    dialogAppInfo.abilityIconId = param.info.iconId;
    dialogAppInfo.abilityLabelId = param.info.labelId;
    dialogAppInfo.bundleIconId = param.info.applicationInfo.iconId;
    dialogAppInfo.bundleLabelId = param.info.applicationInfo.labelId;
    dialogAppInfo.visible = param.info.visible;
    dialogAppInfo.appIndex = param.info.appIndex;
    dialogAppInfo.multiAppMode = param.info.applicationInfo.multiAppMode;
    dialogAppInfo.isAppLink = (param.info.linkType == AppExecFwk::LinkType::APP_LINK);
    dialogAppInfo.codePath = param.info.applicationInfo.codePath;
    dialogAppInfo.installSource = param.info.applicationInfo.installSource;
    dialogAppInfos.emplace_back(dialogAppInfo);
}

bool ImplicitStartProcessor::IsExistDefaultApp(int32_t userId, const std::string &typeName)
{
    auto defaultMgr = GetDefaultAppProxy();
    CHECK_POINTER_AND_RETURN_LOG(defaultMgr, false, "defaultMgr null");
    AppExecFwk::BundleInfo bundleInfo;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId: %{public}d, typeName: %{public}s", userId, typeName.c_str());
    ErrCode ret =
        IN_PROCESS_CALL(defaultMgr->GetDefaultApplication(userId, typeName, bundleInfo));
    if (ret != ERR_OK) {
        return false;
    }

    if (bundleInfo.abilityInfos.size() == 1) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "find default ability");
        return true;
    } else if (bundleInfo.extensionInfos.size() == 1) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "find default extension");
        return true;
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "getDefaultApplication failed");
        return false;
    }
}

void ImplicitStartProcessor::SetTargetLinkInfo(const std::vector<AppExecFwk::SkillUriForAbilityAndExtension> &skillUri,
    Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    for (const auto& iter : skillUri) {
        if (iter.isMatch) {
            want.RemoveParam("send_to_erms_targetLinkFeature");
            want.SetParam("send_to_erms_targetLinkFeature", iter.linkFeature);
            want.RemoveParam("send_to_erms_targetLinkType");
            if (want.GetBoolParam(OPEN_LINK_APP_LINKING_ONLY, false)) {
                want.SetParam("send_to_erms_targetLinkType", AbilityCallerInfo::LINK_TYPE_UNIVERSAL_LINK);
            } else if ((iter.scheme == "https" || iter.scheme == "http") &&
                want.GetAction().compare(ACTION_VIEW) == 0) {
                want.SetParam("send_to_erms_targetLinkType", AbilityCallerInfo::LINK_TYPE_WEB_LINK);
            } else {
                want.SetParam("send_to_erms_targetLinkType", AbilityCallerInfo::LINK_TYPE_DEEP_LINK);
            }
        }
    }
}

bool ImplicitStartProcessor::IsActionImplicitStart(const Want &want, bool findDeafultApp)
{
    std::string supportStart = OHOS::system::GetParameter(SUPPORT_ACTION_START_SELECTOR, "false");
    if (supportStart == "false") {
        return false;
    }

    if (findDeafultApp) {
        return false;
    }

    std::string bundleName = "";
    if (DeepLinkReserveConfig::GetInstance().IsLinkReserved(want.GetUriString(),
        bundleName)) {
        return false;
    }

    if (want.GetUriString() == "" ||
        (want.GetUri().GetScheme() != "file" && want.GetUri().GetScheme() != "content" &&
        want.GetUri().GetScheme() != "mailto")) {
        return false;
    }

    if (want.GetElement().GetBundleName() != "") {
        return false;
    }

    return true;
}

int32_t ImplicitStartProcessor::FindAppClone(std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos, bool &isAppCloneSelector)
{
    if (abilityInfos.size() > 0 && extensionInfos.size() > 0) {
        return ERR_OK;
    }

    bool isExitAbilityAppClone = FindAbilityAppClone(abilityInfos);
    bool isExitExtensionAppClone = FindExtensionAppClone(extensionInfos);
    if ((abilityInfos.size() == 0 && FindExtensionAppClone(extensionInfos)) ||
        (extensionInfos.size() == 0 && FindAbilityAppClone(abilityInfos))) {
        isAppCloneSelector = true;
    }

    return ERR_OK;
}

bool ImplicitStartProcessor::FindAbilityAppClone(std::vector<AppExecFwk::AbilityInfo> &abilityInfos)
{
    if (abilityInfos.size() <= 1) {
        return false;
    }
    std::string appCloneBundleName = "";
    std::string appCloneAbilityName = "";
    for (const auto &iter : abilityInfos) {
        if (appCloneBundleName == "" && appCloneAbilityName == "") {
            appCloneBundleName = iter.bundleName;
            appCloneAbilityName = iter.name;
        }
        if (iter.bundleName != appCloneBundleName || iter.name != appCloneAbilityName) {
            return false;
        }
    }
    return true;
}

void ImplicitStartProcessor::TrustlistIntersectionProcess(const AbilityRequest &request,
    std::vector<DialogAppInfo> &dialogAppInfos, int32_t userId)
{
    if (request.want.GetUri().GetScheme() == FILE_SCHEME_NAME) {
        return;
    }
    std::vector<std::string> appLaunchTrustlist = request.want.GetStringArrayParam(APP_LAUNCH_TRUSTLIST);
    if (appLaunchTrustlist.size() <= 0) {
        return;
    }
    std::vector<DialogAppInfo> dialogIntersectionAppInfos;
    if (appLaunchTrustlist.size() > TRUSTLIST_MAX_SIZE) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "trustlist size is %{public}zu, more than %{public}zu.",
            appLaunchTrustlist.size(), TRUSTLIST_MAX_SIZE);
        appLaunchTrustlist.resize(TRUSTLIST_MAX_SIZE);
    }
    auto bundleMgrHelper = GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrHelper empty");
        return;
    }
    for (const auto &info : dialogAppInfos) {
        AppExecFwk::BundleInfo curBundleInfo;
        bool ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(info.bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, curBundleInfo, userId));
        if (!ret) {
            TAG_LOGW(AAFwkTag::ABILITYMGR,
                "get bundleInfo failed, bundleName:%{public}s, userId:%{public}d.",
                info.bundleName.c_str(), userId);
            continue;
        }
        std::string curAppIdentifier = curBundleInfo.signatureInfo.appIdentifier;
        auto it = std::find(appLaunchTrustlist.begin(), appLaunchTrustlist.end(), curAppIdentifier);
        if (it != appLaunchTrustlist.end()) {
            dialogIntersectionAppInfos.emplace_back(info);
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "trustlist size: %{public}zu, Intersection size: %{public}zu.",
            appLaunchTrustlist.size(), dialogIntersectionAppInfos.size());
    if (dialogIntersectionAppInfos.size() <= 1) {
        dialogAppInfos = dialogIntersectionAppInfos;
    }
    return;
}

bool ImplicitStartProcessor::FindExtensionAppClone(std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos)
{
    if (extensionInfos.size() <= 1) {
        return false;
    }
    std::string appCloneBundleName = "";
    std::string appCloneAbilityName = "";
    for (const auto &iter : extensionInfos) {
        if (appCloneBundleName == "" && appCloneAbilityName == "") {
            appCloneBundleName = iter.bundleName;
            appCloneAbilityName = iter.name;
        }
        if (iter.bundleName != appCloneBundleName || iter.name != appCloneAbilityName) {
            return false;
        }
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS