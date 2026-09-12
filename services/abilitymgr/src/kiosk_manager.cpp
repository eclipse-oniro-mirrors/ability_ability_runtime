/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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


#include <algorithm>

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_record.h"
#include "ability_util.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"
#include "interceptor/kiosk_interceptor.h"
#include "ipc_skeleton.h"
#include "kiosk_manager.h"
#include "permission_constants.h"
#include "session_manager_lite.h"
#include "singleton.h"
#include "utils/want_utils.h"
#include "ws_common.h"

namespace OHOS {
namespace AAFwk {
constexpr char KIOSK_MODE_ENABLED[] = "const.product.kioskmode.enabled";

KioskManager &KioskManager::GetInstance()
{
    static KioskManager manager;
    return manager;
}

void KioskManager::OnAppStop(const AppInfo &info)
{
    if (info.state != AppState::TERMINATED && info.state != AppState::END) {
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "App stop, bundleName: %{public}s, state: %{public}d",
        info.bundleName.c_str(), static_cast<int32_t>(info.state));
    bool shouldExit = false;
    sptr<IRemoteObject> exitToken;
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        if (IsInKioskModeInner() && (info.bundleName == kioskStatus_.kioskBundleName_)) {
            shouldExit = true;
            exitToken = kioskStatus_.kioskToken_;
        }
    }
    if (shouldExit) {
        ExitKioskModeInner(info.bundleName, exitToken, true);
    }
}

int32_t KioskManager::VerifyUpdatePermissions()
{
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Disabled config");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }

    if (!PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not SA call");
        return ERR_NOT_SYSTEM_APP;
    }
    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_EDM_POLICY)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not MANAGE_EDM_POLICY permission");
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}

int32_t KioskManager::VerifyKioskPermissions()
{
    if (!PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not SA call");
        return ERR_NOT_SYSTEM_APP;
    }
    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_SWITCH_KIOSK_MODE)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not SWITCH_KIOSK_MODE permission");
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}

int32_t KioskManager::CheckAndExitIfOutOfList(const std::vector<std::string> &appList)
{
    bool needExit = false;
    std::string exitBundleName;
    sptr<IRemoteObject> exitToken;
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        if (IsInKioskModeInner()) {
            auto it = std::find(appList.begin(), appList.end(), kioskStatus_.kioskBundleName_);
            if (it == appList.end()) {
                needExit = true;
                exitBundleName = kioskStatus_.kioskBundleName_;
                exitToken = kioskStatus_.kioskToken_;
            }
        }
    }
    if (needExit) {
        auto exitRet = ExitKioskModeInner(exitBundleName, exitToken, true);
        if (exitRet != ERR_OK) {
            return exitRet;
        }
    }
    return ERR_OK;
}

int32_t KioskManager::CheckAndExitIfOutOfActiveList(int32_t callerUid, const std::vector<std::string> &appList)
{
    bool needExit = false;
    std::string exitBundleName;
    sptr<IRemoteObject> exitToken;
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        if (IsInKioskModeInner() && kioskStatus_.isProxyEnter_ &&
            callerUid == kioskStatus_.kioskCallerUid_) {
            auto it = std::find(appList.begin(), appList.end(), kioskStatus_.kioskBundleName_);
            if (it == appList.end()) {
                needExit = true;
                exitBundleName = kioskStatus_.kioskBundleName_;
                exitToken = kioskStatus_.kioskToken_;
            }
        }
    }
    if (needExit) {
        auto exitRet = ExitKioskModeInner(exitBundleName, exitToken, true);
        if (exitRet != ERR_OK) {
            return exitRet;
        }
    }
    return ERR_OK;
}

int32_t KioskManager::NotifyWmsUpdateAppList(const std::vector<std::string> &appList)
{
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    CHECK_POINTER_AND_RETURN_LOG(sceneSessionManager, INNER_ERR, "sceneSessionManager is nullptr");
    auto ret = static_cast<int>(sceneSessionManager->UpdateKioskAppList(appList));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UpdateKioskAppList failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t KioskManager::UpdateKioskApplicationList(const std::vector<std::string> &appList)
{
    auto permRet = VerifyUpdatePermissions();
    if (permRet != ERR_OK) {
        return permRet;
    }
    auto exitRet = CheckAndExitIfOutOfList(appList);
    if (exitRet != ERR_OK) {
        return exitRet;
    }
    auto wmsRet = NotifyWmsUpdateAppList(appList);
    if (wmsRet != ERR_OK) {
        return wmsRet;
    }
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        whitelist_.clear();
        for (const auto &app : appList) {
            whitelist_.insert(app);
        }
    }
    return ERR_OK;
}

int32_t KioskManager::AddKioskApplicationList(const std::vector<std::string> &appList)
{
    auto permRet = VerifyKioskPermissions();
    if (permRet != ERR_OK) {
        return permRet;
    }
    if (appList.empty()) {
        return ERR_OK;
    }
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    std::vector<std::string> fullList;
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        auto iter = kioskWhitelistMap_.find(callerUid);
        if (iter == kioskWhitelistMap_.end()) {
            iter = kioskWhitelistMap_.emplace(callerUid, std::unordered_set<std::string>{}).first;
        }
        for (const auto &app : appList) {
            iter->second.insert(app);
        }
        for (const auto &app : iter->second) {
            fullList.push_back(app);
        }
    }
    auto wmsRet = NotifyWmsUpdateAppList(fullList);
    if (wmsRet != ERR_OK) {
        return wmsRet;
    }
    return ERR_OK;
}

int32_t KioskManager::DeleteKioskApplicationList(const std::vector<std::string> &appList)
{
    auto permRet = VerifyKioskPermissions();
    if (permRet != ERR_OK) {
        return permRet;
    }
    if (appList.empty()) {
    // empty appList => no-op, do not clear the caller's slot
        return ERR_OK;
    }
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    std::vector<std::string> remainList;
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        auto it = kioskWhitelistMap_.find(callerUid);
        if (it != kioskWhitelistMap_.end()) {
            for (const auto &app : it->second) {
                if (std::find(appList.begin(), appList.end(), app) == appList.end()) {
                    remainList.push_back(app);
                }
            }
        }
    }
    auto exitRet = CheckAndExitIfOutOfActiveList(callerUid, remainList);
    if (exitRet != ERR_OK) {
        return exitRet;
    }

    auto wmsRet = NotifyWmsUpdateAppList(remainList);
    if (wmsRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyWmsUpdateAppList failed: %{public}d", wmsRet);
        return wmsRet;
    }

    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        auto it = kioskWhitelistMap_.find(callerUid);
        if (it != kioskWhitelistMap_.end()) {
            for (const auto &app : appList) {
                it->second.erase(app);
            }
            if (it->second.empty()) {
                kioskWhitelistMap_.erase(it);
            }
        }
    }
    return ERR_OK;
}

int32_t KioskManager::EnterKioskMode(sptr<IRemoteObject> callerToken, int32_t kioskType)
{
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Disabled config");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    auto record = Token::GetAbilityRecordByToken(callerToken);
    if (!record) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record null");
        return INVALID_PARAMETERS_ERR;
    }
    std::string bundleName = record->GetAbilityInfo().bundleName;
    int32_t recordUid = record->GetAbilityInfo().uid;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    bool isSelf = (callingUid == recordUid);
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        if (isSelf) {
            if (!IsInWhiteListInner(bundleName)) {
                return ERR_KIOSK_MODE_NOT_IN_WHITELIST;
            }
            if (IsInKioskModeInner()) {
                return ERR_ALREADY_IN_KIOSK_MODE;
            }
            kioskStatus_.isProxyEnter_ = false;
        } else {
            auto permRet = VerifyKioskPermissions();
            if (permRet != ERR_OK) {
                return permRet;
            }
            if (!IsSAProxyInKioskWhitelist(callingUid, bundleName)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "proxy target not in caller whitelist");
                return ERR_KIOSK_MODE_NOT_IN_WHITELIST;
            }
            if (IsInKioskModeInner()) {
                return ERR_ALREADY_IN_KIOSK_MODE;
            }
            kioskStatus_.isProxyEnter_ = true;
        }
        kioskStatus_.kioskCallerUid_ = callingUid;
        kioskStatus_.isKioskMode_ = true;
        kioskStatus_.kioskBundleName_ = bundleName;
        kioskStatus_.kioskBundleUid_ = recordUid;
        kioskStatus_.kioskToken_ = callerToken;
        kioskStatus_.kioskType_ = kioskType;
    }
    GetEnterKioskModeCallback()();
    NotifyKioskModeChanged(true, bundleName, recordUid, kioskType);
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    CHECK_POINTER_AND_RETURN_LOG(sceneSessionManager, INNER_ERR, "sceneSessionManager is nullptr");
    sceneSessionManager->EnterKioskMode(callerToken, static_cast<Rosen::KioskType>(kioskType));
    return ERR_OK;
}

int32_t KioskManager::ExitKioskMode(sptr<IRemoteObject> callerToken, bool isFoundation)
{
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Disabled config");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    auto record = Token::GetAbilityRecordByToken(callerToken);
    if (!record) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record null");
        return INVALID_PARAMETERS_ERR;
    }
    return ExitKioskModeInner(record->GetAbilityInfo().bundleName, callerToken, isFoundation);
}

int32_t KioskManager::ExitKioskModeInner(const std::string &bundleName, sptr<IRemoteObject> callerToken,
    bool isFoundation)
{
    std::string outBundleName;
    int32_t outUid = 0;
    int32_t outKioskType = static_cast<int32_t>(Rosen::KioskType::DEFAULT);
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        if (!IsInWhiteListInner(bundleName)) {
            return ERR_KIOSK_MODE_NOT_IN_WHITELIST;
        }
        if (!IsInKioskModeInner()) {
            return ERR_NOT_IN_KIOSK_MODE;
        }
        auto permRet = VerifyKioskPermissions();
        if (!isFoundation && permRet == ERR_OK) {
            if (IPCSkeleton::GetCallingUid() != kioskStatus_.kioskCallerUid_) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is not the SA that entered kiosk");
                return CHECK_PERMISSION_FAILED;
            }
        } else if (!isFoundation && (kioskStatus_.kioskBundleUid_ != IPCSkeleton::GetCallingUid() ||
            kioskStatus_.isProxyEnter_)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName %{public}s is not the currently kiosk app", bundleName.c_str());
            return ERR_NOT_IN_KIOSK_MODE;
        }
        outBundleName = kioskStatus_.kioskBundleName_;
        outUid = kioskStatus_.kioskBundleUid_;
        outKioskType = kioskStatus_.kioskType_;
        kioskStatus_.Clear();
    }
    GetExitKioskModeCallback()();
    NotifyKioskModeChanged(false, outBundleName, outUid, outKioskType);
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    CHECK_POINTER_AND_RETURN_LOG(sceneSessionManager, INNER_ERR, "sceneSessionManager is nullptr");
    sceneSessionManager->ExitKioskMode(callerToken);
    return ERR_OK;
}

int32_t KioskManager::GetKioskStatus(KioskStatus &kioskStatus)
{
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Disabled config");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    if (!PermissionVerification::GetInstance()->IsSystemAppCall() &&
        !(PermissionVerification::GetInstance()->IsSACall() &&
        PermissionVerification::GetInstance()->VerifyCallingPermission(
            PermissionConstants::PERMISSION_GET_EDM_CONFIG))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "permission deny");
        return ERR_NOT_SYSTEM_APP;
    }
    std::lock_guard<std::mutex> lock(kioskManagerMutex_);
    kioskStatus = kioskStatus_;
    return ERR_OK;
}

void KioskManager::FilterDialogAppInfos(std::vector<DialogAppInfo> &dialogAppInfos)
{
    std::lock_guard<std::mutex> lock(kioskManagerMutex_);
    if (!IsInKioskModeInner()) {
        return;
    }
    auto newEnd = std::remove_if(dialogAppInfos.begin(), dialogAppInfos.end(),
        [this](const DialogAppInfo &appInfo) {
            return !IsInWhiteListInner(appInfo.bundleName);
        });
    dialogAppInfos.erase(newEnd, dialogAppInfos.end());
}

void KioskManager::FilterAbilityInfos(std::vector<AppExecFwk::AbilityInfo> &abilityInfos)
{
    std::lock_guard<std::mutex> lock(kioskManagerMutex_);
    if (!IsInKioskModeInner()) {
        return;
    }
    auto newEnd = std::remove_if(abilityInfos.begin(), abilityInfos.end(),
        [this](const AppExecFwk::AbilityInfo &abilityInfo) {
            return !IsInWhiteListInner(abilityInfo.bundleName);
        });
    abilityInfos.erase(newEnd, abilityInfos.end());
}

bool KioskManager::IsInKioskMode()
{
    std::lock_guard<std::mutex> lock(kioskManagerMutex_);
    return IsInKioskModeInner();
}

bool KioskManager::IsInWhiteList(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(kioskManagerMutex_);
    return IsInWhiteListInner(bundleName);
}

bool KioskManager::IsInKioskModeInner()
{
    return kioskStatus_.isKioskMode_;
}

bool KioskManager::IsKioskBundleUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(kioskManagerMutex_);
    return uid == kioskStatus_.kioskBundleUid_;
}

bool KioskManager::ShouldIntercept(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(kioskManagerMutex_);
    return IsInKioskModeInner() && !IsInWhiteListInner(bundleName);
}

void KioskManager::NotifyKioskModeChanged(bool isInKioskMode, const std::string &bundleName,
    int32_t kioskBundleUid, int32_t kioskType)
{
    std::string eventData = isInKioskMode
                                ? EventFwk::CommonEventSupport::COMMON_EVENT_KIOSK_MODE_ON
                                : EventFwk::CommonEventSupport::COMMON_EVENT_KIOSK_MODE_OFF;
    Want want;
    want.SetAction(eventData);
    want.SetParam("bundleName", bundleName);
    want.SetParam("uid", kioskBundleUid);
    want.SetParam("userId", kioskBundleUid / BASE_USER_RANGE);
    want.SetParam("type", kioskType);
    EventFwk::CommonEventData commonData {want};
    if (!IN_PROCESS_CALL(EventFwk::CommonEventManager::PublishCommonEvent(commonData))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PublishCommonEvent failed, eventData: %{public}s", eventData.c_str());
    }
}

bool KioskManager::IsInWhiteListInner(const std::string &bundleName)
{
    // Only the active (current kiosk caller) whitelist participates in interception.
    if (kioskStatus_.isProxyEnter_) {
        return IsSAProxyInKioskWhitelist(kioskStatus_.kioskCallerUid_, bundleName);
    }
    return whitelist_.count(bundleName) != 0;
}

bool KioskManager::IsSAProxyInKioskWhitelist(int32_t callerUid, const std::string &bundleName)
{
    auto it = kioskWhitelistMap_.find(callerUid);
    if (it == kioskWhitelistMap_.end()) {
        return false;
    }
    return it->second.count(bundleName) != 0;
}

std::function<void()> KioskManager::GetEnterKioskModeCallback()
{
    auto enterKioskModeCallback = []() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "EnterKioskMode");
        KioskManager::GetInstance().AddKioskInterceptor();
    };
    return enterKioskModeCallback;
}

std::function<void()> KioskManager::GetExitKioskModeCallback()
{
    auto exitKioskModeCallback = []() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ExitKioskMode");
        KioskManager::GetInstance().RemoveKioskInterceptor();
    };
    return exitKioskModeCallback;
}

void KioskManager::AddKioskInterceptor()
{
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid abilityMgr pointer");
        return;
    }
    auto interceptorExecuter = abilityMgr->GetAbilityInterceptorExecuter();
    if (interceptorExecuter == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid interceptorExecuter pointer");
        return;
    }
    interceptorExecuter->AddInterceptor("KioskWhitelist", std::make_shared<KioskInterceptor>());
}

void KioskManager::RemoveKioskInterceptor()
{
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid abilityMgr pointer");
        return;
    }
    auto interceptorExecuter = abilityMgr->GetAbilityInterceptorExecuter();
    if (interceptorExecuter == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid interceptorExecuter pointer");
        return;
    }
    interceptorExecuter->RemoveInterceptor("KioskWhitelist");
}
} // namespace AAFwk
} // namespace OHOS
