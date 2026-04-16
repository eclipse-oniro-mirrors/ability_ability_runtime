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

#include "interceptor/screen_unlock_interceptor.h"

#include "ability_record.h"
#include "ability_util.h"
#include "bundle_mgr_helper.h"
#include "extension_config.h"
#include "event_report.h"
#include "parameters.h"
#include "start_ability_utils.h"
#include "startup_util.h"
#ifdef SUPPORT_SCREEN
#ifdef ABILITY_RUNTIME_SCREENLOCK_ENABLE
#include "screenlock_manager.h"
#endif // ABILITY_RUNTIME_SCREENLOCK_ENABLE
#endif

namespace OHOS {
namespace AAFwk {

std::string ScreenUnlockInterceptor::GetAppIdentifier(const std::string &bundleName)
{
    if (bundleName.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName is empty");
        return "";
    }
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrHelper is nullptr");
        return "";
    }
    AppExecFwk::SignatureInfo signatureInfo;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetSignatureInfoByBundleName(bundleName, signatureInfo));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSignatureInfoByBundleName failed, bundleName: %{public}s",
            bundleName.c_str());
        return "";
    }
    return signatureInfo.appIdentifier;
}

ErrCode ScreenUnlockInterceptor::DoProcess(AbilityInterceptorParam param)
{
    AppExecFwk::AbilityInfo targetAbilityInfo;
    if (!GetTargetAbilityInfo(param, targetAbilityInfo)) {
        return ERR_OK;
    }

#ifdef SUPPORT_SCREEN
#ifdef ABILITY_RUNTIME_SCREENLOCK_ENABLE
    if (!OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked()) {
        return ERR_OK;
    }
#endif
#endif

    bool isSystemApp = targetAbilityInfo.applicationInfo.isSystemApp;
    if (isSystemApp) {
        return ProcessSystemApp(targetAbilityInfo);
    }
    return ProcessNonSystemApp(targetAbilityInfo);
}

bool ScreenUnlockInterceptor::GetTargetAbilityInfo(const AbilityInterceptorParam &param,
    AppExecFwk::AbilityInfo &targetAbilityInfo)
{
    if (StartAbilityUtils::startAbilityInfo != nullptr) {
        targetAbilityInfo = StartAbilityUtils::startAbilityInfo->abilityInfo;
        return true;
    }
    QueryTargetAbilityInfo(param, targetAbilityInfo);
    if (targetAbilityInfo.applicationInfo.name.empty() ||
        targetAbilityInfo.applicationInfo.bundleName.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Cannot find targetAbilityInfo, element uri: %{public}s/%{public}s",
            param.want.GetElement().GetBundleName().c_str(), param.want.GetElement().GetAbilityName().c_str());
        return false;
    }
    return true;
}

ErrCode ScreenUnlockInterceptor::ProcessSystemApp(const AppExecFwk::AbilityInfo &targetAbilityInfo)
{
    bool allowAppRunWhenDeviceFirstLocked = targetAbilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked;
    if (!allowAppRunWhenDeviceFirstLocked) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "block sys app %{public}s/%{public}s",
            targetAbilityInfo.applicationInfo.bundleName.c_str(), targetAbilityInfo.name.c_str());
        return ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK;
    }

    bool isExtension = targetAbilityInfo.type == AppExecFwk::AbilityType::EXTENSION;
    if (!isExtension) {
        ReportSystemAppUIAbilityEvent(targetAbilityInfo);
        return ERR_OK;
    }

    std::string extensionTypeName = targetAbilityInfo.extensionTypeName;
    std::string bundleName = targetAbilityInfo.applicationInfo.bundleName;
    ErrCode result = CheckExtensionInterception(extensionTypeName, bundleName, true);
    if (result != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "block sys ext %{public}s/%{public}s, type:%{public}s",
            targetAbilityInfo.applicationInfo.bundleName.c_str(),
            targetAbilityInfo.name.c_str(), extensionTypeName.c_str());
    }
    return result;
}

ErrCode ScreenUnlockInterceptor::ProcessNonSystemApp(const AppExecFwk::AbilityInfo &targetAbilityInfo)
{
    bool isExtension = targetAbilityInfo.type == AppExecFwk::AbilityType::EXTENSION;
    if (!isExtension) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "block uiAbility %{public}s/%{public}s",
            targetAbilityInfo.applicationInfo.bundleName.c_str(), targetAbilityInfo.name.c_str());
        return ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK;
    }

    std::string extensionTypeName = targetAbilityInfo.extensionTypeName;
    std::string bundleName = targetAbilityInfo.applicationInfo.bundleName;
    ErrCode result = CheckExtensionInterception(extensionTypeName, bundleName, false);
    if (result != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "block ext %{public}s/%{public}s, type:%{public}s",
            targetAbilityInfo.applicationInfo.bundleName.c_str(),
            targetAbilityInfo.name.c_str(), extensionTypeName.c_str());
    }
    return result;
}

void ScreenUnlockInterceptor::ReportSystemAppUIAbilityEvent(const AppExecFwk::AbilityInfo &targetAbilityInfo)
{
    EventInfo eventInfo;
    eventInfo.bundleName = targetAbilityInfo.applicationInfo.bundleName;
    eventInfo.moduleName = "StartScreenUnlock";
    EventReport::SendStartAbilityOtherExtensionEvent(EventName::START_ABILITY_OTHER_EXTENSION, eventInfo);
}

ErrCode ScreenUnlockInterceptor::CheckExtensionInterception(const std::string &extensionTypeName,
    const std::string &bundleName, bool isSystemApp)
{
    auto extensionConfig = DelayedSingleton<ExtensionConfig>::GetInstance();
    if (!extensionConfig->HasScreenUnlockAccessConfig(extensionTypeName)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "no screen_unlock_access config for extension: %{public}s",
            extensionTypeName.c_str());
        return ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK;
    }

    if (isSystemApp) {
        return CheckSystemAppExtensionInterception(extensionTypeName, bundleName);
    }
    return CheckThirdPartyExtensionInterception(extensionTypeName, bundleName);
}

ErrCode ScreenUnlockInterceptor::CheckSystemAppExtensionInterception(const std::string &extensionTypeName,
    const std::string &bundleName)
{
    auto extensionConfig = DelayedSingleton<ExtensionConfig>::GetInstance();
    bool interception;
    if (extensionConfig->HasScreenUnlockSystemAppInterception(extensionTypeName)) {
        interception = extensionConfig->GetScreenUnlockSystemAppInterception(extensionTypeName);
    } else if (extensionConfig->HasScreenUnlockDefaultInterception(extensionTypeName)) {
        interception = extensionConfig->GetScreenUnlockDefaultInterception(extensionTypeName);
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "no interception config for system app extension");
        return ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK;
    }
    bool needAppIdentifier = interception ? extensionConfig->HasScreenUnlockAccessAllowList(extensionTypeName)
                                          : extensionConfig->HasScreenUnlockAccessBlockList(extensionTypeName);
    std::string appIdentifier;
    if (needAppIdentifier) {
        appIdentifier = GetAppIdentifier(bundleName);
    }
    return CheckInterceptionByConfig(extensionTypeName, appIdentifier, interception, true);
}

ErrCode ScreenUnlockInterceptor::CheckThirdPartyExtensionInterception(const std::string &extensionTypeName,
    const std::string &bundleName)
{
    auto extensionConfig = DelayedSingleton<ExtensionConfig>::GetInstance();
    if (!extensionConfig->HasScreenUnlockDefaultInterception(extensionTypeName)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "no defaultInterception config for third-party extension");
        return ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK;
    }
    bool defaultInterception = extensionConfig->GetScreenUnlockDefaultInterception(extensionTypeName);
    bool needAppIdentifier = defaultInterception ? extensionConfig->HasScreenUnlockAccessAllowList(extensionTypeName)
                                                 : extensionConfig->HasScreenUnlockAccessBlockList(extensionTypeName);
    std::string appIdentifier;
    if (needAppIdentifier) {
        appIdentifier = GetAppIdentifier(bundleName);
    }
    return CheckInterceptionByConfig(extensionTypeName, appIdentifier, defaultInterception, false);
}

ErrCode ScreenUnlockInterceptor::CheckInterceptionByConfig(const std::string &extensionTypeName,
    const std::string &appIdentifier, bool interception, bool isSystemApp)
{
    auto extensionConfig = DelayedSingleton<ExtensionConfig>::GetInstance();
    if (interception) {
        if (extensionConfig->IsInScreenUnlockAccessAllowList(extensionTypeName, appIdentifier)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "app in allowlist, allow");
            return ERR_OK;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "app not in allowlist, block");
        return ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK;
    }
    if (extensionConfig->IsInScreenUnlockAccessBlockList(extensionTypeName, appIdentifier)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "app in blocklist, block");
        return ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "app not in blocklist, allow");
    return ERR_OK;
}

void ScreenUnlockInterceptor::QueryTargetAbilityInfo(const AbilityInterceptorParam &param,
    AppExecFwk::AbilityInfo &targetAbilityInfo)
{
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The bundleMgrHelper is nullptr.");
        return;
    }
    IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(param.want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, param.userId, targetAbilityInfo));
    if (!targetAbilityInfo.applicationInfo.name.empty() && !targetAbilityInfo.applicationInfo.bundleName.empty()) {
        return;
    }

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(param.want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, param.userId, extensionInfos));
    if (extensionInfos.size() <= 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionInfo empty");
        return;
    }
    AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfos.front(), targetAbilityInfo);
}
} // namespace AAFwk
} // namespace OHOS