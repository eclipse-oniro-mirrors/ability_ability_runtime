/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "auto_startup_interface.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "permission_constants.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr char PRODUCT_APPBOOT_SETTING_ENABLED[] = "const.product.appboot.setting.enabled";
} // namespace

AbilityAutoStartupService::AbilityAutoStartupService() {}

AbilityAutoStartupService::~AbilityAutoStartupService() {}

int32_t AbilityAutoStartupService::RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
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
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
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
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    bool isVisible;
    int32_t userId;
    std::string abilityTypeName;
    std::string accessTokenId;
    if (!GetAbilityData(info, isVisible, abilityTypeName, accessTokenId, userId)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get ability data failed");
        return INNER_ERR;
    }

    if (!isVisible) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Current ability not visible");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityTypeName;
    fullInfo.userId = userId;
    fullInfo.accessTokenId = accessTokenId;

    return InnerSetApplicationAutoStartup(fullInfo);
}

int32_t AbilityAutoStartupService::InnerSetApplicationAutoStartup(const AutoStartupInfo &info)
{
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Query auto startup data failed");
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
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Edm application abnormal");
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
    return ERR_ALREADY_EXISTS;
}

int32_t AbilityAutoStartupService::CancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    int32_t code = CheckPermissionForSystem();
    if (code != ERR_OK) {
        return code;
    }

    bool isVisible;
    std::string abilityTypeName;
    std::string accessTokenId;
    int32_t userId;
    if (!GetAbilityData(info, isVisible, abilityTypeName, accessTokenId, userId)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get ability data failed");
        return INNER_ERR;
    }

    if (!isVisible) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Current ability not visible");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = abilityTypeName;
    fullInfo.accessTokenId = accessTokenId;
    fullInfo.userId = userId;

    return InnerCancelApplicationAutoStartup(fullInfo);
}

int32_t AbilityAutoStartupService::InnerCancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Query auto startup data failed");
        return status.code;
    }

    if (status.isEdmForce) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Edm application abnormal");
        return ERR_EDM_APP_CONTROLLED;
    }

    if (status.isAutoStartup) {
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
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    int32_t code = CheckPermissionForEDM();
    code = code == ERR_OK ? code : CheckPermissionForSystem();
    if (code != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Permission verification failed");
        return code;
    }

    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAllAutoStartupApplications(infoList,
        userId);
}

int32_t AbilityAutoStartupService::QueryAllAutoStartupApplicationsWithoutPermission(
    std::vector<AutoStartupInfo> &infoList, int32_t userId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Disabled Product config");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAllAutoStartupApplications(infoList,
        userId);
}

int32_t AbilityAutoStartupService::DeleteAutoStartupData(const std::string &bundleName, const int32_t uid)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(bundleName, uid);
}

int32_t AbilityAutoStartupService::CheckAutoStartupData(const std::string &bundleName, int32_t uid)
{
    int32_t userId;
    int32_t appIndex = 0;
    AppExecFwk::BundleInfo bundleInfo;
    if (!GetBundleInfo(bundleName, bundleInfo, uid, userId, appIndex)) {
        return INNER_ERR;
    }
    auto tokenId = bundleInfo.applicationInfo.accessTokenId;
    std::string accessTokenIdStr = std::to_string(tokenId);
    std::vector<AutoStartupInfo> infoList;
    int32_t result = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->GetCurrentAppAutoStartupData(
        bundleName, infoList, accessTokenIdStr);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get auto startup data failed");
        return result;
    }
    if (infoList.size() == 0) {
        return ERR_OK;
    }

    bool isFound = false;
    for (auto info : infoList) {
        for (auto abilityInfo : bundleInfo.abilityInfos) {
            if ((abilityInfo.bundleName == info.bundleName) && (abilityInfo.name == info.abilityName) &&
                (info.moduleName.empty() || (abilityInfo.moduleName == info.moduleName))) {
                isFound = true;
                break;
            }
        }
    }

    if (!isFound) {
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Current bundleName not found");
        return DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->DeleteAutoStartupData(bundleName, uid);
    }
    return ERR_OK;
}

void AbilityAutoStartupService::ExecuteCallbacks(bool isCallOn, const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    for (auto item : callbackVector_) {
        auto remoteSystemCallback = iface_cast<IAutoStartupCallBack>(item);
        if (remoteSystemCallback != nullptr) {
            if (isCallOn) {
                remoteSystemCallback->OnAutoStartupOn(info);
            } else {
                remoteSystemCallback->OnAutoStartupOff(info);
            }
        }
    }

    auto it = callbackMaps_.find(info.bundleName);
    if (it != callbackMaps_.end()) {
        auto remoteCallback = iface_cast<IAutoStartupCallBack>(it->second);
        if (remoteCallback != nullptr) {
            if (isCallOn) {
                remoteCallback->OnAutoStartupOn(info);
            } else {
                remoteCallback->OnAutoStartupOff(info);
            }
        }
    }
}

void AbilityAutoStartupService::SetDeathRecipient(
    const sptr<IRemoteObject> &callback, const sptr<IRemoteObject::DeathRecipient> &deathRecipient)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (callback == nullptr || deathRecipient == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "CallerToken or deathRecipient empty");
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
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    auto object = remote.promote();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null remote obj");
        return;
    }

    // Clean the callbackVector_ and callbackMaps_.
    {
        std::lock_guard<std::mutex> lock(autoStartUpMutex_);
        for (auto item = callbackVector_.begin(); item != callbackVector_.end();) {
            if (*item == object) {
                item = callbackVector_.erase(item);
            } else {
                item++;
            }
        }

        for (auto it = callbackMaps_.begin(); it != callbackMaps_.end();) {
            const auto &callback = it->second;
            if (callback == object) {
                it = callbackMaps_.erase(it);
            } else {
                it++;
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
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
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
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get BundleMgrClient failed");
        return "";
    }

    std::string bundleName;
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (IN_PROCESS_CALL(bundleMgrClient->GetNameForUid(callerUid, bundleName)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get Bundle Name failed");
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

bool AbilityAutoStartupService::GetBundleInfo(const std::string &bundleName,
    AppExecFwk::BundleInfo &bundleInfo, int32_t uid, int32_t &userId, int32_t appIndex)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");

    if (uid == -1) {
        userId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
    } else {
        userId = uid / AppExecFwk::Constants::BASE_USER_RANGE;
    }
    if (userId == 0) {
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null abilityMgr");
            return false;
        }
        userId = abilityMgr->GetUserId();
    }
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "userId: %{public}d", userId);
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
                TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get bundle info failed");
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
                TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetSandboxBundleInfo failed");
                return false;
            }
    }
    return true;
}

bool AbilityAutoStartupService::GetAbilityData(const AutoStartupInfo &info, bool &isVisible,
    std::string &abilityTypeName, std::string &accessTokenId, int32_t &userId)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s,"
        " accessTokenId: %{public}s, userId: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(),
        info.abilityName.c_str(), info.accessTokenId.c_str(), info.userId);
    AppExecFwk::BundleInfo bundleInfo;
    int32_t currentUserId;
    int32_t uid = bundleInfo.applicationInfo.uid;
    if (!GetBundleInfo(info.bundleName, bundleInfo, uid, currentUserId, info.appCloneIndex)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to GetBundleInfo.");
        return false;
    }
    userId = currentUserId;
    auto accessTokenIdStr = bundleInfo.applicationInfo.accessTokenId;
    accessTokenId = std::to_string(accessTokenIdStr);
    for (const auto& hapModuleInfo : bundleInfo.hapModuleInfos) {
        for (const auto& abilityInfo : hapModuleInfo.abilityInfos) {
            if ((abilityInfo.bundleName == info.bundleName) && (abilityInfo.name == info.abilityName) &&
                (info.moduleName.empty() || (abilityInfo.moduleName == info.moduleName))) {
                isVisible = abilityInfo.visible;
                abilityTypeName = GetAbilityTypeName(abilityInfo);
                TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Get ability info success");
                return true;
            }
        }
    }

    for (auto extensionInfo : bundleInfo.extensionInfos) {
        if ((extensionInfo.bundleName == info.bundleName) && (extensionInfo.name == info.abilityName)) {
            if (info.moduleName.empty() || (extensionInfo.moduleName == info.moduleName)) {
                isVisible = extensionInfo.visible;
                abilityTypeName = GetExtensionTypeName(extensionInfo);
                TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Get extension info success");
                return true;
            }
        }
    }
    return false;
}

std::string AbilityAutoStartupService::GetAbilityTypeName(AppExecFwk::AbilityInfo abilityInfo)
{
    std::string abilityTypeName;
    if (abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        abilityTypeName = "UIAbility";
    }
    return abilityTypeName;
}

std::string AbilityAutoStartupService::GetExtensionTypeName(AppExecFwk::ExtensionAbilityInfo extensionInfo)
{
    std::string abilityTypeName;
    if (extensionInfo.type == AppExecFwk::ExtensionAbilityType::SERVICE) {
        abilityTypeName = "ServiceExtension";
    }
    return abilityTypeName;
}

std::shared_ptr<AppExecFwk::BundleMgrClient> AbilityAutoStartupService::GetBundleMgrClient()
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (bundleMgrClient_ == nullptr) {
        bundleMgrClient_ = DelayedSingleton<AppExecFwk::BundleMgrClient>::GetInstance();
    }
    return bundleMgrClient_;
}

int32_t AbilityAutoStartupService::CheckPermissionForSystem()
{
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Disabled Product config");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    if (!PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Caller is not system-app, can not use system-api");
        return ERR_NOT_SYSTEM_APP;
    }

    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_APP_BOOT)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, " Verify PERMISSION_MANAGE_APP_BOOT failed");
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::CheckPermissionForSelf(const std::string &bundleName)
{
    if (!system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Disabled Product config");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    if (!CheckSelfApplication(bundleName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Not self application");
        return ERR_NOT_SELF_APPLICATION;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupService::GetAbilityInfo(
    const AutoStartupInfo &info, std::string &abilityTypeName, std::string &accessTokenId, int32_t &userId)
{
    bool isVisible = false;
    if (!GetAbilityData(info, isVisible, abilityTypeName, accessTokenId, userId)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get ability data failed");
        return INNER_ERR;
    }

    if (!isVisible) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Current ability not visible");
        return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    }

    return ERR_OK;
}

int32_t AbilityAutoStartupService::SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    int32_t errorCode = CheckPermissionForEDM();
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    int32_t userId;
    std::string typeName;
    std::string accessTokenId;

    errorCode = GetAbilityInfo(info, typeName, accessTokenId, userId);
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = typeName;
    fullInfo.accessTokenId = accessTokenId;
    fullInfo.userId = userId;
    return InnerApplicationAutoStartupByEDM(fullInfo, true, flag);
}

int32_t AbilityAutoStartupService::CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    int32_t errorCode = CheckPermissionForEDM();
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    int32_t userId;
    std::string typeName;
    std::string accessTokenId;
    errorCode = GetAbilityInfo(info, typeName, accessTokenId, userId);
    if (errorCode != ERR_OK) {
        return errorCode;
    }
    AutoStartupInfo fullInfo(info);
    fullInfo.abilityTypeName = typeName;
    fullInfo.accessTokenId = accessTokenId;
    fullInfo.userId = userId;
    return InnerApplicationAutoStartupByEDM(fullInfo, false, flag);
}

int32_t AbilityAutoStartupService::InnerApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool isSet, bool flag)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP,
        "Called, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s, accessTokenId: %{public}s,"
        " userId: %{public}d, isSet: %{public}d, flag: %{public}d",
        info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str(),
        info.accessTokenId.c_str(), info.userId, isSet, flag);
    AutoStartupStatus status =
        DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance()->QueryAutoStartupData(info);
    if (status.code != ERR_OK && status.code != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Query auto startup data failed");
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
        return result;
    }

    return result;
}

int32_t AbilityAutoStartupService::CheckPermissionForEDM()
{
    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_APP_BOOT_INTERNAL)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Verify ohos.permission.MANAGE_APP_BOOT_INTERNAL failed");
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
