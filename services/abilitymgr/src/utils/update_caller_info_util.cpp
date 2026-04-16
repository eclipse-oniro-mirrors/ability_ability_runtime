/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "utils/update_caller_info_util.h"

#include "ability_event_util.h"
#include "ability_util.h"
#include "ability_record.h"
#include "accesstoken_kit.h"
#include "app_scheduler.h"
#include "ams_configuration_parameter.h"
#include "dialog_session_manager.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "global_constant.h"
#include "permission_verification.h"
#include "scene_board_judgement.h"
#include "start_ability_utils.h"
#include "startup_util.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* DMS_CALLER_BUNDLE_NAME = "ohos.dms.param.sourceCallerBundleName";
constexpr const char* DMS_CALLER_ABILITY_NAME = "ohos.dms.param.sourceCallerAbilityName";
constexpr const char* DMS_CALLER_NATIVE_NAME = "ohos.dms.param.sourceCallerNativeName";
constexpr const char* DMS_CALLER_APP_ID = "ohos.dms.param.sourceCallerAppId";
constexpr const char* DMS_CALLER_APP_IDENTIFIER = "ohos.dms.param.sourceCallerAppIdentifier";
constexpr const char* PARAM_RESV_ANCO_CALLER_UID = "ohos.anco.param.callerUid";
constexpr const char* PARAM_RESV_ANCO_CALLER_BUNDLENAME = "ohos.anco.param.callerBundleName";
constexpr const char* WANT_PARAMS_APP_RESTART_FLAG = "ohos.aafwk.app.restart";
constexpr const char* CALLER_REQUEST_CODE = "ohos.extra.param.key.callerRequestCode";
constexpr const char* IS_SHELL_CALL = "isShellCall";
constexpr const char* COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
constexpr const char* SPECIFIED_ABILITY_FLAG = "ohos.ability.params.specifiedAbilityFlag";
constexpr const char* SHELL_ASSISTANT_BUNDLENAME = "com.ohos.shell_assistant";
constexpr const char* UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
constexpr int32_t BROKER_UID = 5557;
}

UpdateCallerInfoUtil &UpdateCallerInfoUtil::GetInstance()
{
    static UpdateCallerInfoUtil instance;
    return instance;
}

void UpdateCallerInfoUtil::UpdateByIndirectCallerInfo(std::shared_ptr<IndirectCallerInfo> indirectCallerInfo,
    int32_t &tokenId, int32_t &callerUid, int32_t &callerPid)
{
    if (indirectCallerInfo != nullptr) {
        tokenId = indirectCallerInfo->tokenId;
        callerUid = indirectCallerInfo->callerUid;
        callerPid = indirectCallerInfo->callerPid;
    }
}

void UpdateCallerInfoUtil::UpdateCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken,
    std::shared_ptr<IndirectCallerInfo> indirectCallerInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!StartAbilityUtils::IsCallFromAncoShellOrBroker(callerToken)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not call from anco or broker.");
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_UID);
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_BUNDLENAME);
    }
    ClearProtectedWantParam(want);
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    UpdateByIndirectCallerInfo(indirectCallerInfo, tokenId, callerUid, callerPid);
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);
    want.RemoveParam(WANT_PARAMS_APP_RESTART_FLAG);
    want.RemoveParam(IS_SHELL_CALL);
    want.RemoveParam(Want::PARAMS_REAL_CALLER_KEY);
    want.RemoveParam(AbilityRuntime::GlobalConstant::GAME_PRELAUNCH);

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (!abilityRecord) {
        std::string bundleName;
        auto bundleMgr = AbilityUtil::GetBundleManagerHelper();
        if (bundleMgr != nullptr) {
            IN_PROCESS_CALL(bundleMgr->GetNameForUid(callerUid, bundleName));
        }
        if (bundleName == "") {
            std::string nativeName;
            Security::AccessToken::NativeTokenInfo nativeTokenInfo;
            int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, nativeTokenInfo);
            if (result == ERR_OK) {
                nativeName = "_" + nativeTokenInfo.processName;
            }
            want.SetParam(Want::PARAM_RESV_CALLER_NATIVE_NAME, nativeName);
        }
        UpdateCallerBundleName(want, bundleName);
        UpdateCallerAbilityName(want, "");
        UpdateCallerAppCloneIndex(want, 0);
        return;
    }
    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    UpdateCallerBundleName(want, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    UpdateCallerAbilityName(want, callerAbilityName);
    UpdateCallerAppCloneIndex(want, abilityRecord->GetAppIndex());
    UpdateSignatureInfo(callerBundleName, want);
}

void UpdateCallerInfoUtil::UpdateSignatureInfo(std::string bundleName, Want& want, bool isRemote)
{
    auto bundleMgr = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgr != nullptr) {
        AppExecFwk::SignatureInfo signatureInfo;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName: %{public}s", bundleName.c_str());
        IN_PROCESS_CALL(bundleMgr->GetSignatureInfoByBundleName(bundleName, signatureInfo));
        std::string callerAppId = isRemote ? DMS_CALLER_APP_ID : Want::PARAM_RESV_CALLER_APP_ID;
        std::string callerAppIdentifier = isRemote ? DMS_CALLER_APP_IDENTIFIER : Want::PARAM_RESV_CALLER_APP_IDENTIFIER;
        want.RemoveParam(callerAppId);
        want.SetParam(callerAppId, signatureInfo.appId);
        want.RemoveParam(callerAppIdentifier);
        want.SetParam(callerAppIdentifier, signatureInfo.appIdentifier);
    }
}

void UpdateCallerInfoUtil::UpdateAsCallerSourceInfo(Want& want, sptr<IRemoteObject> asCallerSourceToken,
    sptr<IRemoteObject> callerToken)
{
    if (callerToken != nullptr && !StartAbilityUtils::IsCallFromAncoShellOrBroker(callerToken)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not call from anco or broker.");
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_UID);
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_BUNDLENAME);
    }
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    want.RemoveParam(WANT_PARAMS_APP_RESTART_FLAG);
    want.RemoveParam(IS_SHELL_CALL);
    want.RemoveParam(Want::PARAMS_REAL_CALLER_KEY);
    want.RemoveParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX);
    want.RemoveParam(AbilityRuntime::GlobalConstant::GAME_PRELAUNCH);
    ClearProtectedWantParam(want);
#ifdef SUPPORT_SCREEN
    if (UpdateAsCallerInfoFromDialog(want)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Update as caller source info from dialog.");
        return;
    }
#endif // SUPPORT_SCREEN
    if (AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Update as caller source info from token.");
        UpdateAsCallerInfoFromToken(want, asCallerSourceToken);
    } else if (callerToken != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Update as caller source info from callerRecord.");
        UpdateAsCallerInfoFromCallerRecord(want, callerToken);
    }
}

void UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken(Want& want, sptr<IRemoteObject> asCallerSourceToken)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(asCallerSourceToken);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed update caller info from token");
        return;
    }
    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(asCallerSourceToken, processInfo);
    int32_t tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, processInfo.uid_);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, processInfo.pid_);

    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
    UpdateCallerAppCloneIndex(want, abilityRecord->GetAppIndex());
    UpdateSignatureInfo(callerBundleName, want);
}

void UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord(Want& want, sptr<IRemoteObject> callerToken)
{
    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER(callerRecord);
    auto sourceInfo = callerRecord->GetCallerInfo();
    CHECK_POINTER(sourceInfo);
    std::string callerBundleName = sourceInfo->callerBundleName;
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, sourceInfo->callerTokenId);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, sourceInfo->callerUid);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, sourceInfo->callerPid);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, sourceInfo->callerAbilityName);
    want.SetParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, sourceInfo->callerAppCloneIndex);
    if (callerBundleName == "") {
        want.SetParam(Want::PARAM_RESV_CALLER_NATIVE_NAME, sourceInfo->callerNativeName);
        return;
    }
    UpdateSignatureInfo(callerBundleName, want);
}

bool UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog(Want& want)
{
    std::string dialogSessionId = want.GetStringParam("dialogSessionId");
    auto dialogCallerInfo = DialogSessionManager::GetInstance().GetDialogCallerInfo(dialogSessionId);
    auto removeParamsForNotBroker = [&want](int32_t uid, const std::string& callerBundleName) {
        if (uid != BROKER_UID && callerBundleName != SHELL_ASSISTANT_BUNDLENAME) {
            want.RemoveParam(PARAM_RESV_ANCO_CALLER_UID);
            want.RemoveParam(PARAM_RESV_ANCO_CALLER_BUNDLENAME);
        }
    };
    if (dialogCallerInfo == nullptr) {
        removeParamsForNotBroker(IPCSkeleton::GetCallingUid(),
            want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME));
        TAG_LOGW(AAFwkTag::ABILITYMGR, "failed get dialog caller info");
        return false;
    }
    Want dialogCallerWant = dialogCallerInfo->targetWant;
    int32_t tokenId = dialogCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0);
    int32_t uid = dialogCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0);
    int32_t pid = dialogCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0);
    std::string callerBundleName = dialogCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    std::string callerAbilityName = dialogCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    int32_t callerAppCloneIndex = dialogCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, 0);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, uid);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, pid);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
    want.SetParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, callerAppCloneIndex);

    removeParamsForNotBroker(uid, callerBundleName);
    if (callerBundleName == "") {
        want.SetParam(Want::PARAM_RESV_CALLER_NATIVE_NAME,
            dialogCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME));
        return true;
    }
    UpdateSignatureInfo(callerBundleName, want);
    return true;
}

void UpdateCallerInfoUtil::UpdateCallerInfoFromToken(Want& want, const sptr<IRemoteObject> &token)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "caller abilityRecord null");
        return;
    }
    ClearProtectedWantParam(want);
    int32_t tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    int32_t callerUid = abilityRecord->GetUid();
    int32_t callerPid = abilityRecord->GetPid();
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);
    want.RemoveParam(WANT_PARAMS_APP_RESTART_FLAG);
    want.RemoveParam(IS_SHELL_CALL);
    want.RemoveParam(Want::PARAMS_REAL_CALLER_KEY);
    want.RemoveParam(AbilityRuntime::GlobalConstant::GAME_PRELAUNCH);

    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
    if (abilityRecord->GetApplicationInfo().bundleName != AbilityConfig::SCENEBOARD_BUNDLE_NAME ||
        AppExecFwk::ConvertToExtensionAbilityType(want.GetStringParam(UIEXTENSION_TYPE_KEY)) !=
        AppExecFwk::ExtensionAbilityType::EMBEDDED_UI) {
        UpdateCallerAppCloneIndex(want, abilityRecord->GetAppIndex());
    }
    UpdateSignatureInfo(callerBundleName, want);
}

void UpdateCallerInfoUtil::UpdateBackToCallerFlag(const sptr<IRemoteObject> &callerToken, Want &want,
    int32_t requestCode, bool backFlag)
{
    want.RemoveParam(AbilityRuntime::GlobalConstant::GAME_PRELAUNCH);
    if (want.HasParameter(CALLER_REQUEST_CODE)) {
        want.RemoveParam(CALLER_REQUEST_CODE);
    }
    auto callerAbilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (requestCode > 0 && callerAbilityRecord != nullptr) {
        // default return true on oh
        if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            backFlag = AmsConfigurationParameter::GetInstance().IsSupportBackToCaller();
        }
        auto fullRequestCode = AbilityRuntime::StartupUtil::GenerateFullRequestCode(
            callerAbilityRecord->GetPid(), backFlag, requestCode);
        want.SetParam(CALLER_REQUEST_CODE, std::to_string(fullRequestCode));
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "pid: %{public}d, backFlag:%{private}d, requestCode: %{private}d, fullRequestCode: %{private}s",
            callerAbilityRecord->GetPid(), backFlag, requestCode, std::to_string(fullRequestCode).c_str());
    }
}

void UpdateCallerInfoUtil::UpdateDmsCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    ClearProtectedWantParam(want);
    want.RemoveParam(AbilityRuntime::GlobalConstant::GAME_PRELAUNCH);

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (!abilityRecord) {
        std::string bundleName;
        auto bundleMgr = AbilityUtil::GetBundleManagerHelper();
        if (bundleMgr != nullptr) {
            IN_PROCESS_CALL(bundleMgr->GetNameForUid(callerUid, bundleName));
        }
        if (bundleName == "") {
            std::string nativeName;
            Security::AccessToken::NativeTokenInfo nativeTokenInfo;
            int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, nativeTokenInfo);
            if (result == ERR_OK) {
                nativeName = "_" + nativeTokenInfo.processName;
            }
            want.RemoveParam(DMS_CALLER_NATIVE_NAME);
            want.SetParam(DMS_CALLER_NATIVE_NAME, nativeName);
        }
        want.RemoveParam(DMS_CALLER_BUNDLE_NAME);
        want.SetParam(DMS_CALLER_BUNDLE_NAME, bundleName);
        want.RemoveParam(DMS_CALLER_ABILITY_NAME);
        want.SetParam(DMS_CALLER_ABILITY_NAME, std::string(""));
        return;
    }
    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    want.RemoveParam(DMS_CALLER_BUNDLE_NAME);
    want.SetParam(DMS_CALLER_BUNDLE_NAME, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    want.RemoveParam(DMS_CALLER_ABILITY_NAME);
    want.SetParam(DMS_CALLER_ABILITY_NAME, callerAbilityName);
    UpdateSignatureInfo(callerBundleName, want, true);
}

void UpdateCallerInfoUtil::UpdateCallerBundleName(Want& want, const std::string &bundleName)
{
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, bundleName);
}

void UpdateCallerInfoUtil::UpdateCallerAbilityName(Want& want, const std::string &abilityName)
{
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, abilityName);
}

void UpdateCallerInfoUtil::UpdateCallerAppCloneIndex(Want& want, int32_t appIndex)
{
    want.RemoveParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX);
    want.SetParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, appIndex);
}

void UpdateCallerInfoUtil::ClearProtectedWantParam(Want &want)
{
    if (want.HasParameter(Want::PARAM_RESV_CALLER_NATIVE_NAME)) {
        EventInfo eventInfo;
        eventInfo.bundleName = want.GetElement().GetBundleName();
        eventInfo.moduleName = want.GetElement().GetModuleName();
        eventInfo.abilityName = want.GetElement().GetAbilityName();
        int32_t callerUid = IPCSkeleton::GetCallingUid();
        std::string callerBundleName;
        auto bundleMgr = AbilityUtil::GetBundleManagerHelper();
        if (bundleMgr != nullptr) {
            IN_PROCESS_CALL(bundleMgr->GetNameForUid(callerUid, callerBundleName));
        }
        eventInfo.callerBundleName = callerBundleName.empty() ? std::to_string(callerUid) : callerBundleName;
        AbilityEventUtil::SendStartAbilityErrorEvent(eventInfo, AAFwk::ERR_NOT_EXPECTED_NATIVE_CALLER_NAME,
            std::string("no expected caller native name: ") + want.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME));
    }
    want.RemoveParam(Want::PARAM_RESV_CALLER_NATIVE_NAME);
    want.RemoveParam(COMPONENT_STARTUP_NEW_RULES);
    if (!PermissionVerification::GetInstance()->IsSystemAppCall()) {
        // only system app can set unifiedDataKey
        want.RemoveParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY);
    }
    want.RemoveParam(SPECIFIED_ABILITY_FLAG);
}
}  // namespace AAFwk
}  // namespace OHOS
