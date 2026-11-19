/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ui_extension_record_factory.h"

#include "hilog_tag_wrapper.h"
#include "multi_instance_utils.h"
#include "ui_extension_record.h"
#include "permission_verification.h"
#include "ability_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
}

UIExtensionRecordFactory::UIExtensionRecordFactory() = default;

UIExtensionRecordFactory::~UIExtensionRecordFactory() = default;

bool UIExtensionRecordFactory::NeedReuse(const AAFwk::AbilityRequest &abilityRequest, int32_t &extensionRecordId)
{
    auto sessionInfo = abilityRequest.sessionInfo;
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null sessionInfo, no reuse");
        return false;
    }
    int32_t uiExtensionAbilityId = sessionInfo->want.GetIntParam(UIEXTENSION_ABILITY_ID,
        INVALID_EXTENSION_RECORD_ID);
    if (uiExtensionAbilityId == INVALID_EXTENSION_RECORD_ID) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "UIEXTENSION_ABILITY_ID is not config, no reuse");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "UIExtensionAbility id: %{public}d.", uiExtensionAbilityId);
    extensionRecordId = uiExtensionAbilityId;
    return true;
}


bool UIExtensionRecordFactory::IsInAllowList(const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos,
    const std::string &extensionName, const std::string &callerAppIdentifier)
{
    for (const AppExecFwk::ExtensionAbilityInfo &info : extensionInfos) {
        if (info.type == AppExecFwk::ExtensionAbilityType::EMBEDDED_UI && info.name == extensionName) {
            auto it = std::find_if(info.appIdentifierAllowList.begin(),
                info.appIdentifierAllowList.end(),
                [callerAppIdentifier](const std::string &allowAppId) {
                    return allowAppId == callerAppIdentifier || allowAppId == "allow_all";
                });
            return it != info.appIdentifierAllowList.end();
        }
    }
    return false;
}

bool UIExtensionRecordFactory::CheckAllowCrossUserEmbeddedUI(
    const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifySupportCrossAppEmbedForOaPermission()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "no permission to cross user embeddedUI");
        return false;
    }

    auto bms = AAFwk::AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, false);
    std::string callerAppIdentifier = abilityRequest.want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER);
    if (callerAppIdentifier.empty()) {
        AppExecFwk::SignatureInfo signatureInfo;
        if (IN_PROCESS_CALL(bms->GetSignatureInfoByBundleName(hostBundleName, signatureInfo)) != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "bms GetSignatureInfoByBundleName error, bundleName: %{public}s",
                hostBundleName.c_str());
            return false;
        };
        callerAppIdentifier = signatureInfo.appIdentifier;
    }
    AppExecFwk::BundleInfo targetBundleInfo;
    std::string targetBundleName = abilityRequest.abilityInfo.bundleName;
    if (!IN_PROCESS_CALL(bms->GetBundleInfo(targetBundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO,
        targetBundleInfo,
        abilityRequest.userId))) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "bms GetBundleInfo error, BundleFlag: GET_BUNDLE_WITH_EXTENSION_INFO");
            return false;
    }

    if (IsInAllowList(targetBundleInfo.extensionInfos, abilityRequest.abilityInfo.name, callerAppIdentifier)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "allow cross user embeddedUI bundleName:%{public}s", hostBundleName.c_str());
        return true;
    }
    return false;
}

int32_t UIExtensionRecordFactory::CheckHostBundleName(
    const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName)
{
    if (hostBundleName == abilityRequest.abilityInfo.applicationName ||
        CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName) ||
        (abilityRequest.isTargetPlugin && abilityRequest.hostBundleName == hostBundleName)) {
        return ERR_OK;
    }

    TAG_LOGW(AAFwkTag::ABILITYMGR, "not called");
    return ERR_INVALID_VALUE;
}

int32_t UIExtensionRecordFactory::EmbeddedUIPreCheck(
    const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName)
{
    if (hostBundleName == AAFwk::AbilityConfig::SCENEBOARD_BUNDLE_NAME) {
        return ERR_OK;
    }

    auto result = CheckHostBundleName(abilityRequest, hostBundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PreCheck error:%{public}d", result);
        return result;
    }

    // There may exist preload extension, the session info is nullptr
    if (abilityRequest.sessionInfo != nullptr) {
        auto callerToken = abilityRequest.sessionInfo->callerToken;
        auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(callerToken);
        CHECK_POINTER_AND_RETURN(callerAbilityRecord, ERR_INVALID_VALUE);
        AppExecFwk::AbilityInfo abilityInfo = callerAbilityRecord->GetAbilityInfo();
        if (abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "not UIAbility");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

int32_t UIExtensionRecordFactory::PreCheck(
    const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName)
{
    auto result = ExtensionRecordFactory::PreCheck(abilityRequest, hostBundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PreCheck error:%{public}d", result);
        return result;
    }
    if (abilityRequest.abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::EMBEDDED_UI) {
        result = EmbeddedUIPreCheck(abilityRequest, hostBundleName);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "EmbeddedUIPreCheck error:%{public}d", result);
            return result;
        }
    }

    return ERR_OK;
}

int32_t UIExtensionRecordFactory::CreateRecord(
    const AAFwk::AbilityRequest &abilityRequest, std::shared_ptr<ExtensionRecord> &extensionRecord)
{
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create record failed");
        return ERR_NULL_OBJECT;
    }
    if (abilityRequest.extensionType == AppExecFwk::ExtensionAbilityType::EMBEDDED_UI &&
        abilityRequest.sessionInfo != nullptr) {
        auto callerToken = abilityRequest.sessionInfo->callerToken;
        auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(callerToken);
        if (callerAbilityRecord != nullptr) {
            int32_t appIndex = callerAbilityRecord->GetAppIndex();
            abilityRecord->SetAppIndex(appIndex);
            abilityRecord->SetWantAppIndex(appIndex);
        }
    }
    if (AAFwk::MultiInstanceUtils::IsMultiInstanceApp(abilityRequest.appInfo)) {
        abilityRecord->SetInstanceKey(AAFwk::MultiInstanceUtils::GetValidExtensionInstanceKey(abilityRequest));
    }
    CreateDebugRecord(abilityRequest, abilityRecord);
    extensionRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    uint32_t processMode = GetExtensionProcessMode(abilityRequest, extensionRecord->isHostSpecified_);
    extensionRecord->processMode_ = processMode;
    abilityRecord->SetExtensionProcessMode(processMode);
    return ERR_OK;
}

void UIExtensionRecordFactory::CreateDebugRecord(
    const AAFwk::AbilityRequest &abilityRequest, std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord)
{
    auto callerRecord = AAFwk::Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (!callerRecord) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No caller record");
        return;
    }
    if (!callerRecord->IsDebug() ||
        callerRecord->GetApplicationInfo().appProvisionType !=
        AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Not debug UIExtension");
            return;
    }
    auto callerBundleName = callerRecord->GetAbilityInfo().bundleName;
    auto isSameApp = callerBundleName == abilityRequest.abilityInfo.bundleName;
    auto isCallerUIAbility = callerRecord->GetAbilityInfo().type == AppExecFwk::AbilityType::PAGE;
    if (isSameApp && isCallerUIAbility) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Setting up debug UIExtension");
        abilityRecord->SetDebugUIExtension();
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
