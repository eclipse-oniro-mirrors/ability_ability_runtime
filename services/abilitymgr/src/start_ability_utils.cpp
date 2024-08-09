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

#include "start_ability_utils.h"

#include "ability_record.h"
#include "ability_util.h"
#include "app_utils.h"
#include "global_constant.h"
#include "hitrace_meter.h"
#include "startup_util.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* SCREENSHOT_BUNDLE_NAME = "com.huawei.ohos.screenshot";
constexpr const char* SCREENSHOT_ABILITY_NAME = "com.huawei.ohos.screenshot.ServiceExtAbility";
constexpr int32_t ERMS_ISALLOW_RESULTCODE = 10;
constexpr const char* PARAM_RESV_ANCO_CALLER_UID = "ohos.anco.param.callerUid";
constexpr const char* PARAM_RESV_ANCO_CALLER_BUNDLENAME = "ohos.anco.param.callerBundleName";
constexpr int32_t REQUEST_CODE_LENGTH = 32;
constexpr int32_t PID_LENGTH = 16;
constexpr int32_t REQUEST_CODE_PID_LENGTH = 48;
}
thread_local std::shared_ptr<StartAbilityInfo> StartAbilityUtils::startAbilityInfo;
thread_local std::shared_ptr<StartAbilityInfo> StartAbilityUtils::callerAbilityInfo;
thread_local bool StartAbilityUtils::skipCrowTest = false;
thread_local bool StartAbilityUtils::skipStartOther = false;
thread_local bool StartAbilityUtils::skipErms = false;
thread_local int32_t StartAbilityUtils::ermsResultCode = ERMS_ISALLOW_RESULTCODE;
thread_local bool StartAbilityUtils::isWantWithAppCloneIndex = false;

bool StartAbilityUtils::GetAppIndex(const Want &want, sptr<IRemoteObject> callerToken, int32_t &appIndex)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && abilityRecord->GetAppIndex() > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX &&
        abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        appIndex = abilityRecord->GetAppIndex();
        return true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "appCloneIndex: %{public}d.", want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0));
    return AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex);
}

bool StartAbilityUtils::GetApplicationInfo(const std::string &bundleName, int32_t userId,
    AppExecFwk::ApplicationInfo &appInfo)
{
    if (StartAbilityUtils::startAbilityInfo &&
        StartAbilityUtils::startAbilityInfo->GetAppBundleName() == bundleName) {
        appInfo = StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo;
    } else {
        if (bundleName.empty()) {
            return false;
        }
        auto bms = AbilityUtil::GetBundleManagerHelper();
        CHECK_POINTER_AND_RETURN(bms, false);
        bool result = IN_PROCESS_CALL(
            bms->GetApplicationInfo(bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
                userId, appInfo)
        );
        if (!result) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Get app info from bms failed: %{public}s", bundleName.c_str());
            return false;
        }
    }
    return true;
}

bool StartAbilityUtils::GetCallerAbilityInfo(const sptr<IRemoteObject> &callerToken,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    if (StartAbilityUtils::callerAbilityInfo) {
        abilityInfo = StartAbilityUtils::callerAbilityInfo->abilityInfo;
    } else {
        if (callerToken == nullptr) {
            return false;
        }
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        if (abilityRecord == nullptr) {
            return false;
        }
        abilityInfo = abilityRecord->GetAbilityInfo();
    }
    return true;
}

int32_t StartAbilityUtils::CheckAppProvisionMode(const Want& want, int32_t userId)
{
    auto abilityInfo = StartAbilityUtils::startAbilityInfo;
    if (!abilityInfo || abilityInfo->GetAppBundleName() != want.GetElement().GetBundleName()) {
        int32_t appIndex = 0;
        if (!AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid app clone index");
            return ERR_APP_CLONE_INDEX_INVALID;
        }
        abilityInfo = StartAbilityInfo::CreateStartAbilityInfo(want, userId, appIndex);
    }
    CHECK_POINTER_AND_RETURN(abilityInfo, GET_ABILITY_SERVICE_FAILED);
    if (abilityInfo->status != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "unexpected abilityInfo status=%{public}d", abilityInfo->status);
        return abilityInfo->status;
    }
    if ((abilityInfo->abilityInfo).applicationInfo.appProvisionType !=
        AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "window options are not supported in non-app-provision mode.");
        return ERR_NOT_IN_APP_PROVISION_MODE;
    }
    return ERR_OK;
}

std::vector<int32_t> StartAbilityUtils::GetCloneAppIndexes(const std::string &bundleName, int32_t userId)
{
    std::vector<int32_t> appIndexes;
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, appIndexes);
    IN_PROCESS_CALL_WITHOUT_RET(bms->GetCloneAppIndexes(bundleName, appIndexes, userId));
    return appIndexes;
}

StartAbilityInfoWrap::StartAbilityInfoWrap(const Want &want, int32_t validUserId, int32_t appIndex,
    const sptr<IRemoteObject> &callerToken, bool isExtension)
{
    if (StartAbilityUtils::startAbilityInfo != nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "startAbilityInfo has been created");
    }
    // This is for special goal and could be removed later.
    auto element = want.GetElement();
    if (element.GetAbilityName() == SCREENSHOT_ABILITY_NAME &&
        element.GetBundleName() == SCREENSHOT_BUNDLE_NAME) {
        isExtension = true;
        StartAbilityUtils::skipErms = true;
    }
    Want localWant = want;
    if (!StartAbilityUtils::IsCallFromAncoShellOrBroker(callerToken)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not call from anco or broker.");
        localWant.RemoveParam(PARAM_RESV_ANCO_CALLER_UID);
        localWant.RemoveParam(PARAM_RESV_ANCO_CALLER_BUNDLENAME);
        localWant.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
        localWant.RemoveParam(Want::PARAM_RESV_CALLER_UID);
        localWant.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
        localWant.SetParam(Want::PARAM_RESV_CALLER_TOKEN, static_cast<int32_t>(IPCSkeleton::GetCallingTokenID()));
        localWant.SetParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    }
    if (isExtension) {
        StartAbilityUtils::startAbilityInfo = StartAbilityInfo::CreateStartExtensionInfo(localWant,
            validUserId, appIndex);
    } else {
        StartAbilityUtils::startAbilityInfo = StartAbilityInfo::CreateStartAbilityInfo(localWant,
            validUserId, appIndex);
    }
    if (StartAbilityUtils::startAbilityInfo != nullptr &&
        StartAbilityUtils::startAbilityInfo->abilityInfo.type == AppExecFwk::AbilityType::EXTENSION) {
        StartAbilityUtils::skipCrowTest = true;
        StartAbilityUtils::skipStartOther = true;
    }

    if (StartAbilityUtils::callerAbilityInfo != nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callerAbilityInfo has been created");
    }
    StartAbilityUtils::callerAbilityInfo = StartAbilityInfo::CreateCallerAbilityInfo(callerToken);

    StartAbilityUtils::ermsResultCode = ERMS_ISALLOW_RESULTCODE;
    StartAbilityUtils::isWantWithAppCloneIndex = false;
    if (want.HasParameter(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY) && appIndex >= 0 &&
        appIndex < AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        StartAbilityUtils::isWantWithAppCloneIndex = true;
    }
}

StartAbilityInfoWrap::~StartAbilityInfoWrap()
{
    StartAbilityUtils::startAbilityInfo.reset();
    StartAbilityUtils::callerAbilityInfo.reset();
    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::skipStartOther = false;
    StartAbilityUtils::skipErms = false;
    StartAbilityUtils::ermsResultCode = ERMS_ISALLOW_RESULTCODE;
    StartAbilityUtils::isWantWithAppCloneIndex = false;
}

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::CreateStartAbilityInfo(const Want &want, int32_t userId,
    int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, nullptr);
    auto abilityInfoFlag = static_cast<uint32_t>(AbilityRuntime::StartupUtil::BuildAbilityInfoFlag()) |
        static_cast<uint32_t>(AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL);
    auto request = std::make_shared<StartAbilityInfo>();
    if (appIndex > 0 && appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryCloneAbilityInfo(want.GetElement(), abilityInfoFlag, appIndex,
            request->abilityInfo, userId));
        if (request->abilityInfo.name.empty() || request->abilityInfo.bundleName.empty()) {
            FindExtensionInfo(want, abilityInfoFlag, userId, appIndex, request);
        }
        return request;
    }
    if (appIndex == 0) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryAbilityInfo(want, abilityInfoFlag, userId, request->abilityInfo));
    } else {
        IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxAbilityInfo(want, appIndex,
            abilityInfoFlag, userId, request->abilityInfo));
    }
    if (request->abilityInfo.name.empty() || request->abilityInfo.bundleName.empty()) {
        // try to find extension
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        if (appIndex == 0) {
            IN_PROCESS_CALL_WITHOUT_RET(bms->QueryExtensionAbilityInfos(want, abilityInfoFlag,
                userId, extensionInfos));
        } else {
            IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxExtAbilityInfos(want, appIndex,
                abilityInfoFlag, userId, extensionInfos));
        }
        if (extensionInfos.size() <= 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Get extension info failed.");
            request->status = RESOLVE_ABILITY_ERR;
            return request;
        }

        AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
        if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionInfo empty.");
            request->status = RESOLVE_ABILITY_ERR;
            return request;
        }
        request->extensionProcessMode = extensionInfo.extensionProcessMode;
        // For compatibility translates to AbilityInfo
        AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, request->abilityInfo);
    }
    return request;
}

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::CreateStartExtensionInfo(const Want &want, int32_t userId,
    int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, nullptr);
    auto abilityInfoFlag = static_cast<uint32_t>(AbilityRuntime::StartupUtil::BuildAbilityInfoFlag()) |
        static_cast<uint32_t>(AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL);
    auto abilityInfo = std::make_shared<StartAbilityInfo>();
    if (appIndex > 0 && appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        FindExtensionInfo(want, abilityInfoFlag, userId, appIndex, abilityInfo);
        return abilityInfo;
    }

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    if (appIndex == 0) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryExtensionAbilityInfos(want, abilityInfoFlag, userId, extensionInfos));
    } else {
        IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxExtAbilityInfos(want, appIndex,
            abilityInfoFlag, userId, extensionInfos));
    }
    if (extensionInfos.size() <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CreateStartExtensionInfo error. Get extension info failed.");
        abilityInfo->status = RESOLVE_ABILITY_ERR;
        return abilityInfo;
    }

    AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
    if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionInfo empty.");
        abilityInfo->status = RESOLVE_ABILITY_ERR;
        return abilityInfo;
    }
    abilityInfo->extensionProcessMode = extensionInfo.extensionProcessMode;
    // For compatibility translates to AbilityInfo
    AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo->abilityInfo);

    return abilityInfo;
}

void StartAbilityInfo::FindExtensionInfo(const Want &want, int32_t flags, int32_t userId,
    int32_t appIndex, std::shared_ptr<StartAbilityInfo> abilityInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_LOG(abilityInfo, "abilityInfo is invalid.");
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_LOG(bms, "bms is invalid.");
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    IN_PROCESS_CALL_WITHOUT_RET(bms->QueryCloneExtensionAbilityInfoWithAppIndex(want.GetElement(),
        flags, appIndex, extensionInfo, userId));
    if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionInfo empty.");
        abilityInfo->status = RESOLVE_ABILITY_ERR;
        return;
    }
    if (AbilityRuntime::StartupUtil::IsSupportAppClone(extensionInfo.type)) {
        abilityInfo->extensionProcessMode = extensionInfo.extensionProcessMode;
        // For compatibility translates to AbilityInfo
        AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo->abilityInfo);
    } else {
        abilityInfo->status = ERR_APP_CLONE_INDEX_INVALID;
    }
}

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::CreateCallerAbilityInfo(const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (callerToken == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not call from context.");
        return nullptr;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can not find abilityRecord");
        return nullptr;
    }
    auto request = std::make_shared<StartAbilityInfo>();
    request->abilityInfo = abilityRecord->GetAbilityInfo();
    return request;
}

bool StartAbilityUtils::IsCallFromAncoShellOrBroker(const sptr<IRemoteObject> &callerToken)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid == AppUtils::GetInstance().GetCollaboratorBrokerUID()) {
        return true;
    }
    AppExecFwk::AbilityInfo callerAbilityInfo;
    if (GetCallerAbilityInfo(callerToken, callerAbilityInfo)) {
        return callerAbilityInfo.bundleName == AppUtils::GetInstance().GetShellAssistantBundleName();
    }
    return false;
}

int64_t StartAbilityUtils::GenerateFullRequestCode(int32_t pid, bool backFlag, int32_t requestCode)
{
    if (requestCode <= 0 || pid <= 0) {
        return 0;
    }
    int64_t fullRequestCode = requestCode;
    uint64_t tempNum = pid;
    fullRequestCode |= (tempNum << REQUEST_CODE_LENGTH);
    if (backFlag) {
        tempNum = 1;
        fullRequestCode |= (tempNum << REQUEST_CODE_PID_LENGTH);
    }
    return fullRequestCode;
}

CallerRequestInfo StartAbilityUtils::ParseFullRequestCode(int64_t fullRequestCode)
{
    CallerRequestInfo requestInfo;
    if (fullRequestCode <= 0) {
        return requestInfo;
    }
    uint64_t tempNum = 1;
    requestInfo.requestCode = (fullRequestCode & ((tempNum << REQUEST_CODE_LENGTH) - 1));
    fullRequestCode >>= REQUEST_CODE_LENGTH;
    requestInfo.pid = (fullRequestCode & ((tempNum << PID_LENGTH) - 1));
    fullRequestCode >>= PID_LENGTH;
    requestInfo.backFlag = (fullRequestCode == 1);
    return requestInfo;
}
}
}