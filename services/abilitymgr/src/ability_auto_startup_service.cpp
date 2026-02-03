/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "ability_auto_startup_service.h"

#include "ability_auto_startup_data_manager.h"
#include "ability_manager_service.h"
#include "app_utils.h"
#include "auto_startup_callback_proxy.h"
#include "auto_startup_interface.h"
#include "display_util.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "permission_constants.h"
#include "server_constant.h"
#include "user_controller/user_controller.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;

AbilityAutoStartupService::AbilityAutoStartupService() {}

AbilityAutoStartupService::~AbilityAutoStartupService() {}

int32_t AbilityAutoStartupService::RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        bool isFound = false;
        auto item = callbackVector_.begin();
        while (item != callbackVector_.end()) {
            if (*item == callback) {
                isFound = true;
                break;
            }
            item++;
        }
        if (!isFound) {
            callbackVector_.emplace_back(callback);
            SetDeathRecipient(
                callback, new (std::nothrow) AbilityAutoStartupService::ClientDeathRecipient(weak_from_this()));
        } else {
            TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Callback already exist");
        }
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        bool isFound = false;
        auto item = callbackVector_.begin();
        while (item != callbackVector_.end()) {
            if (*item == callback) {
                item = callbackVector_.erase(item);
                isFound = true;
            } else {
                item++;
            }
        }
        if (!isFound) {
            TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Callback not exist");
        }
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::SetApplicationAutoStartup(const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, setterUserId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.setterUserId);
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    AutoStartupAbilityData abilityData;
    code = GetAbilityInfo(info, abilityData);
    if (code != ERR_OK) {
        return code;
    }

    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityData.abilityTypeName;
    fullInfo.setterUserId = abilityData.setterUserId;
    fullInfo.accessTokenId = abilityData.accessTokenId;
    fullInfo.userId = abilityData.userId;
    fullInfo.canUserModify = true;
    fullInfo.setterType = AutoStartupSetterType::USER;

    return InnerSetApplicationAutoStartup(fullInfo);
}

int32_t AbilityAutoStartupService::InnerSetApplicationAutoStartup(const AutoStartupInfo &info)
{
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "QueryAutoStartupData fail");
        return status.code;
    }

    int32_t result;
    if (status.code == ERR_NAME_NOT_FOUND) {
        TAG_LOGI(AAFwkTag::AUTO_STARTUP, "Not found");
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->InsertAutoStartupData(info, true, false);
        if (result == ERR_OK) {
            ExecuteCallbacks(true, info);
        }
        return result;
    }
    if (status.isEdmForce) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Edm abnormal");
        return ERR_EDM_APP_CONTROLLED;
    }
    if (!status.isAutoStartup) {
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->UpdateAutoStartupData(info, true, false);
        if (result == ERR_OK) {
            ExecuteCallbacks(true, info);
        }
        return result;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::CancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, setterUserId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.setterUserId);
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    AutoStartupAbilityData abilityData;
    code = GetAbilityInfo(info, abilityData);
    if (code != ERR_OK) {
        return code;
    }

    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityData.abilityTypeName;
    fullInfo.accessTokenId = abilityData.accessTokenId;
    fullInfo.setterUserId = abilityData.setterUserId;
    fullInfo.userId = abilityData.userId;
    fullInfo.canUserModify = true;
    fullInfo.setterType = AutoStartupSetterType::USER;

    return InnerCancelApplicationAutoStartup(fullInfo);
}

int32_t AbilityAutoStartupService::InnerCancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "QueryAutoStartupData fail");
        return status.code;
    }

    if (status.isEdmForce) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Edm abnormal");
        return ERR_EDM_APP_CONTROLLED;
    }

    if (status.isAutoStartup) {
        if (status.setterUserId != -1 &&
            status.setterUserId != info.setterUserId &&
            status.setterType == AutoStartupSetterType::USER) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "setter id is different, cannot cancel");
            return ERR_INVALID_OPERATION;
        }
        int32_t result = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(info);
        if (result == ERR_OK) {
            ExecuteCallbacks(false, info);
        }
        return result;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList,
    int32_t userId)
{
    int32_t code = CheckPermissionForEDM();
    bool isCalledByEDM = (code == ERR_OK);
    if (!isCalledByEDM) {
        code = CheckPermissionForSystem();
    }
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "permission verification fail");
        return code;
    }

    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAllAutoStartupApplications(infoList,
        userId, isCalledByEDM);
}

int32_t AbilityAutoStartupService::GetAutoStartupStatusForSelf(uint32_t callerTokenId, bool &isAutoStartEnabled)
{
    if (!AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Disabled config");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }

    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->GetAutoStartupStatusForSelf(callerTokenId,
        isAutoStartEnabled);
}

int32_t AbilityAutoStartupService::QueryAllAutoStartupApplicationsWithoutPermission(
    std::vector<AutoStartupInfo> &infoList, int32_t userId)
{
    if (!AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Disabled config");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAllAutoStartupApplications(infoList,
        userId, false);
}

int32_t AbilityAutoStartupService::DeleteAutoStartupData(const std::string &bundleName, const int32_t accessTokenId)
{
    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(
        bundleName, accessTokenId);
}

int32_t AbilityAutoStartupService::CheckAutoStartupData(const std::string &bundleName, int32_t uid)
{
    int32_t appIndex = 0;
    AppExecFwk::BundleInfo bundleInfo;
    int32_t userId = uid / AppExecFwk::Constants::BASE_USER_RANGE;
    int32_t validUserId = GetValidUserId(userId);
    if (!GetBundleInfo(bundleName, validUserId, appIndex, bundleInfo)) {
        return INNER_ERR;
    }
    auto tokenId = bundleInfo.applicationInfo.accessTokenId;
    std::string accessTokenIdStr = std::to_string(tokenId);
    std::vector<AutoStartupInfo> infoList;
    int32_t result = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->GetCurrentAppAutoStartupData(
        bundleName, infoList, accessTokenIdStr);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "get auto startup data fail");
        return result;
    }
    if (infoList.size() == 0) {
        return ERR_OK;
    }

    bool isFound = false;
    for (auto info : infoList) {
        for (auto abilityInfo : bundleInfo.abilityInfos) {
            if (IsTargetAbility(info, abilityInfo)) {
                isFound = true;
                break;
            }
        }
    }

    if (!isFound) {
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Current bundleName not found");
        return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(bundleName,
            tokenId);
    }
    return ERR_OK;
}

void AbilityAutoStartupService::GetCallbackVector(std::vector<sptr<IRemoteObject>>& callbackVector)
{
    std::lock_guard<std::mutex> lock(autoStartUpMutex_);
    callbackVector = callbackVector_;
}

void AbilityAutoStartupService::ExecuteCallbacks(bool isCallOn, const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, setterUserId: %{public}d, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.setterUserId, info.userId);
    bool isUserIdMatch = UserController::GetInstance().IsForegroundUser(info.userId);
    bool isUserIdU0OrU1 = (U0_USER_ID == info.userId) || (U1_USER_ID == info.userId);
    if (!isUserIdMatch && !isUserIdU0OrU1) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Condition not satisfied");
        return;
    }
    std::vector<sptr<IRemoteObject>> callbackVector;
    GetCallbackVector(callbackVector);
    for (auto& item : callbackVector) {
        auto remoteSystemCallback = iface_cast<IAutoStartupCallBack>(item);
        if (remoteSystemCallback != nullptr) {
            if (isCallOn) {
                remoteSystemCallback->OnAutoStartupOn(info);
            } else {
                remoteSystemCallback->OnAutoStartupOff(info);
            }
        }
    }
}

void AbilityAutoStartupService::SetDeathRecipient(
    const sptr<IRemoteObject> &callback, const sptr<IRemoteObject::DeathRecipient> &deathRecipient)
{
    if (callback == nullptr || deathRecipient == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "callerToken or deathRecipient empty");
        return;
    }
    std::lock_guard<std::mutex> lock(deathRecipientsMutex_);
    auto iter = deathRecipients_.find(callback);
    if (iter == deathRecipients_.end()) {
        deathRecipients_.emplace(callback, deathRecipient);
        callback->AddDeathRecipient(deathRecipient);
        return;
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "DeathRecipient added");
}

void AbilityAutoStartupService::CleanResource(const wptr<IRemoteObject> &remote)
{
    auto object = remote.promote();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null remote obj");
        return;
    }

    // Clean the callbackVector_.
    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        for (auto item = callbackVector_.begin(); item != callbackVector_.end();) {
            if (*item == object) {
                item = callbackVector_.erase(item);
            } else {
                item++;
            }
        }
    }
    {
        std::lock_guard<std::mutex> deathLock(deathRecipientsMutex_);
        auto iter = deathRecipients_.find(object);
        if (iter != deathRecipients_.end()) {
            auto deathRecipient = iter->second;
            deathRecipients_.erase(iter);
            object->RemoveDeathRecipient(deathRecipient);
        }
    }
}

AbilityAutoStartupService::ClientDeathRecipient::ClientDeathRecipient(
    const std::weak_ptr<AbilityAutoStartupService> &weakPtr)
{
    weakPtr_ = weakPtr;
}

void AbilityAutoStartupService::ClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto abilityAutoStartupService = weakPtr_.lock();
    if (abilityAutoStartupService == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null abilityAutoStartupService");
        return;
    }
    abilityAutoStartupService->CleanResource(remote);
}

std::string AbilityAutoStartupService::GetSelfApplicationBundleName()
{
    auto bundleMgrClient = GetBundleMgrClient();
    if (bundleMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null bundleMgrClient");
        return "";
    }

    std::string bundleName;
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (IN_PROCESS_CALL(bundleMgrClient->GetNameForUid(callerUid, bundleName)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "get bundleName fail");
        return "";
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Get bundle name: %{public}s", bundleName.c_str());
    return bundleName;
}

bool AbilityAutoStartupService::CheckSelfApplication(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Called, bundleName: %{public}s", bundleName.c_str());
    return GetSelfApplicationBundleName() == bundleName ? true : false;
}

int32_t AbilityAutoStartupService::GetValidUserId(int32_t userId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "userId = %{public}d.", userId);
    int32_t validUserId = userId;

    if (userId == -1) {
        validUserId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    }
    if (validUserId == U0_USER_ID || validUserId == U1_USER_ID) {
        validUserId = UserController::GetInstance().GetForegroundUserId(ServerConstant::DEFAULT_DISPLAY_ID);
    }
    return validUserId;
}

bool AbilityAutoStartupService::GetBundleInfo(const std::string &bundleName, int32_t userId, int32_t appIndex,
    AppExecFwk::BundleInfo &bundleInfo)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "bundleName: %{public}s, userId: %{public}d, appIndex: %{public}d",
        bundleName.c_str(), userId, appIndex);
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null bundleMgrHelper");
        return false;
    }
    if (appIndex == 0) {
        auto flags =
            AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES | AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
        if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(
            bundleName, static_cast<AppExecFwk::BundleFlag>(flags), bundleInfo, userId))) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "get bundleInfo fail");
            return false;
        }
    } else if (appIndex <= GlobalConstant::MAX_APP_CLONE_INDEX) {
        auto bundleFlag = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
        auto bundleMgrResult = IN_PROCESS_CALL(
            bundleMgrHelper->GetCloneBundleInfo(bundleName, bundleFlag, appIndex, bundleInfo, userId));
        if (bundleMgrResult != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "error bundleMgrResult");
            return false;
        }
    } else {
        if (!IN_PROCESS_CALL(bundleMgrHelper->GetSandboxBundleInfo(bundleName, appIndex, userId, bundleInfo))) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetSandboxBundleInfo fail");
            return false;
        }
    }
    return true;
}

bool AbilityAutoStartupService::GetAbilityData(const AutoStartupInfo &info, AutoStartupAbilityData &abilityData)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, setterUserId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.setterUserId);
    AppExecFwk::BundleInfo bundleInfo;
    int32_t validUserId = GetValidUserId(info.userId);
    if (!GetBundleInfo(info.bundleName, validUserId, info.appCloneIndex, bundleInfo)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetBundleInfo fail");
        return false;
    }
    auto accessTokenIdStr = bundleInfo.applicationInfo.accessTokenId;
    abilityData.accessTokenId = std::to_string(accessTokenIdStr);
    abilityData.setterUserId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    abilityData.userId = bundleInfo.applicationInfo.uid / AppExecFwk::Constants::BASE_USER_RANGE;
    for (const auto& hapModuleInfo : bundleInfo.hapModuleInfos) {
        for (const auto& abilityInfo : hapModuleInfo.abilityInfos) {
            if (IsTargetAbility(info, abilityInfo)) {
                abilityData.isVisible = abilityInfo.visible;
                abilityData.abilityTypeName = GetAbilityTypeName(abilityInfo);
                return true;
            }
        }
    }

    for (const auto& extensionInfo : bundleInfo.extensionInfos) {
        if (IsTargetExtension(info, extensionInfo)) {
            abilityData.isVisible = extensionInfo.visible;
            abilityData.abilityTypeName = GetExtensionTypeName(extensionInfo);
            return true;
        }
    }
    return false;
}

bool AbilityAutoStartupService::IsTargetAbility(const AutoStartupInfo &info,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    return ((abilityInfo.bundleName == info.bundleName) &&
           (abilityInfo.name == info.abilityName) &&
           (info.moduleName.empty() || abilityInfo.moduleName == info.moduleName));
}

bool AbilityAutoStartupService::IsTargetExtension(const AutoStartupInfo &info,
    const AppExecFwk::ExtensionAbilityInfo &extensionInfo)
{
    return ((extensionInfo.bundleName == info.bundleName) &&
           (extensionInfo.name == info.abilityName) &&
           (info.moduleName.empty() || extensionInfo.moduleName == info.moduleName));
}

std::string AbilityAutoStartupService::GetAbilityTypeName(const AppExecFwk::AbilityInfo &abilityInfo)
{
    if (abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        return "UIAbility";
    }
    return "";
}

std::string AbilityAutoStartupService::GetExtensionTypeName(
    const AppExecFwk::ExtensionAbilityInfo &extensionInfo)
{
    switch (extensionInfo.type) {
        case AppExecFwk::ExtensionAbilityType::APP_SERVICE:
            return EXTENSION_TYPE_APP_SERVICE;
        case AppExecFwk::ExtensionAbilityType::SERVICE:
            return "ServiceExtension";
        default:
            return "";
    }
}

std::shared_ptr<AppExecFwk::BundleMgrClient> AbilityAutoStartupService::GetBundleMgrClient()
{
    if (bundleMgrClient_ == nullptr) {
        bundleMgrClient_ = DelayedSingleton<AppExecFwk::BundleMgrClient>::GetInstance();
    }
    return bundleMgrClient_;
}

int32_t AbilityAutoStartupService::CheckPermissionForSystem()
{
    if (!AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Disabled config");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    if (!PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not use system-api");
        return ERR_NOT_SYSTEM_APP;
    }

    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_APP_BOOT)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, " verify PERMISSION_MANAGE_APP_BOOT fail");
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::CheckPermissionForSelf(const std::string &bundleName)
{
    if (!AAFwk::AppUtils::GetInstance().IsProductAppbootSettingEnabled()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Disabled config");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    if (!CheckSelfApplication(bundleName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Not self application");
        return ERR_NOT_SELF_APPLICATION;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::GetAbilityInfo(
    const AutoStartupInfo &info, AutoStartupAbilityData &abilityData)
{
    if (!GetAbilityData(info, abilityData)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetAbilityData fail");
        return INNER_ERR;
    }

    if (!abilityData.isVisible) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not visible");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    if ((abilityData.abilityTypeName == EXTENSION_TYPE_APP_SERVICE) &&
        (abilityData.userId != U1_USER_ID)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "AppServiceExtension does not belong to U1");
        return INNER_ERR;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    int32_t errorCode = CheckPermissionForEDM();
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    AutoStartupAbilityData abilityData;
    errorCode = GetAbilityInfo(info, abilityData);
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityData.abilityTypeName;
    fullInfo.accessTokenId = abilityData.accessTokenId;
    fullInfo.setterUserId = abilityData.setterUserId;
    fullInfo.userId = abilityData.userId;
    fullInfo.canUserModify = !flag;
    fullInfo.setterType = AutoStartupSetterType::SYSTEM;
    return InnerApplicationAutoStartupByEDM(fullInfo, true, flag);
}

int32_t AbilityAutoStartupService::CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    int32_t errorCode = CheckPermissionForEDM();
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    AutoStartupAbilityData abilityData;
    errorCode = GetAbilityInfo(info, abilityData);
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityData.abilityTypeName;
    fullInfo.accessTokenId = abilityData.accessTokenId;
    fullInfo.setterUserId = abilityData.setterUserId;
    fullInfo.userId = abilityData.userId;
    fullInfo.canUserModify = !flag;
    fullInfo.setterType = AutoStartupSetterType::SYSTEM;
    return InnerApplicationAutoStartupByEDM(fullInfo, false, flag);
}

int32_t AbilityAutoStartupService::InnerApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool isSet, bool flag)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s, accessTokenId: %{public}s,"
        " setterUserId: %{public}d, isSet: %{public}d, flag: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str(),
        info.accessTokenId.c_str(), info.setterUserId, isSet, flag);
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "QueryAutoStartupData fail");
        return status.code;
    }

    int32_t result = ERR_OK;
    if (status.code == ERR_NAME_NOT_FOUND) {
        result = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->InsertAutoStartupData(
            info, isSet, flag);
        if (result == ERR_OK && isSet) {
            ExecuteCallbacks(isSet, info);
        }
        return result;
    }

    bool isFlag = isSet ? !status.isAutoStartup : status.isAutoStartup;
    if (isFlag) {
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->UpdateAutoStartupData(info, isSet, flag);
        if (result == ERR_OK) {
            ExecuteCallbacks(isSet, info);
        }
        return result;
    }
    if (status.isEdmForce != flag) {
        result =
            DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->UpdateAutoStartupData(info, isSet, flag);
        if (result == ERR_OK) {
            ExecuteCallbacks(isSet, info);
        }
        return result;
    }

    return result;
}

int32_t AbilityAutoStartupService::CheckPermissionForEDM()
{
    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_APP_BOOT_INTERNAL)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "verify PERMISSION_MANAGE_APP_BOOT_INTERNAL fail");
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}

void AbilityAutoStartupService::AddHandledAutoStartupUsers(int32_t userId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "AddhandledAutoStartupUsers userId:%{public}d", userId);
    std::lock_guard<std::mutex> lock(autoStartUpMutex_);
    for (int32_t user : handledAutoStartupUsers_) {
        if (user == userId) {
            TAG_LOGD(AAFwkTag::AUTO_STARTUP, "already has userId:%{public}d", userId);
            return;
        }
    }
    handledAutoStartupUsers_.emplace_back(userId);
}

void AbilityAutoStartupService::RemoveHandledAutoStartupUsers(int32_t userId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "RemoveHandledAutoStartupUsers userId:%{public}d", userId);
    std::lock_guard<std::mutex> lock(autoStartUpMutex_);
    handledAutoStartupUsers_.erase(
        std::remove(handledAutoStartupUsers_.begin(), handledAutoStartupUsers_.end(), userId),
        handledAutoStartupUsers_.end());
}

bool AbilityAutoStartupService::FindHandledAutoStartupUsers(int32_t userId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "FindHandledAutoStartupUsers userId:%{public}d", userId);
    std::lock_guard<std::mutex> lock(autoStartUpMutex_);
    for (int32_t user : handledAutoStartupUsers_) {
        if (user == userId) {
            TAG_LOGI(AAFwkTag::AUTO_STARTUP, "find userId:%{public}d", userId);
            return true;
        }
    }
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS
