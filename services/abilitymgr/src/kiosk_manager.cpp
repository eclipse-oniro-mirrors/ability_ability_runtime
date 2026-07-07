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

int32_t KioskManager::UpdateKioskApplicationList(const std::vector<std::string> &appList)
{
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Disabled config");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }

    if (!PermissionVerification::GetInstance()->IsSystemAppCall() &&
        !PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not system app");
        return ERR_NOT_SYSTEM_APP;
    }
    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_EDM_POLICY)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not MANAGE_EDM_POLICY permission");
        return CHECK_PERMISSION_FAILED;
    }
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
        auto ret = ExitKioskModeInner(exitBundleName, exitToken, true);
        if (ret != ERR_OK) {
            return ret;
        }
    }
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        whitelist_.clear();
        for (const auto &app : appList) {
            whitelist_.insert(app);
        }
    }
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    CHECK_POINTER_AND_RETURN_LOG(sceneSessionManager, INNER_ERR, "sceneSessionManager is nullptr");
    sceneSessionManager->UpdateKioskAppList(appList);
    return ERR_OK;
}

int32_t KioskManager::EnterKioskMode(sptr<IRemoteObject> callerToken)
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
    if (!CheckCallerIsForeground(callerToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The application is not in the foreground !");
        return ERR_APP_NOT_IN_FOCUS;
    }
    int32_t kioskBundleUid = IPCSkeleton::GetCallingUid();
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        if (!IsInWhiteListInner(bundleName)) {
            return ERR_KIOSK_MODE_NOT_IN_WHITELIST;
        }

        if (IsInKioskModeInner()) {
            return ERR_ALREADY_IN_KIOSK_MODE;
        }

        kioskStatus_.isKioskMode_ = true;
        kioskStatus_.kioskBundleName_ = bundleName;
        kioskStatus_.kioskBundleUid_ = kioskBundleUid;
        kioskStatus_.kioskToken_ = callerToken;
    }
    GetEnterKioskModeCallback()();
    NotifyKioskModeChanged(true, bundleName, kioskBundleUid);
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    CHECK_POINTER_AND_RETURN_LOG(sceneSessionManager, INNER_ERR, "sceneSessionManager is nullptr");
    sceneSessionManager->EnterKioskMode(callerToken);
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
    {
        std::lock_guard<std::mutex> lock(kioskManagerMutex_);
        if (!IsInWhiteListInner(bundleName)) {
            return ERR_KIOSK_MODE_NOT_IN_WHITELIST;
        }

        if (!IsInKioskModeInner()) {
            return ERR_NOT_IN_KIOSK_MODE;
        }

        if (!isFoundation && kioskStatus_.kioskBundleUid_ != IPCSkeleton::GetCallingUid()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName %{public}s is not the currently kiosk app", bundleName.c_str());
            return ERR_NOT_IN_KIOSK_MODE;
        }

        outBundleName = kioskStatus_.kioskBundleName_;
        outUid = kioskStatus_.kioskBundleUid_;
        kioskStatus_.Clear();
    }
    GetExitKioskModeCallback()();
    NotifyKioskModeChanged(false, outBundleName, outUid);
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

void KioskManager::NotifyKioskModeChanged(bool isInKioskMode, const std::string &bundleName,
    int32_t kioskBundleUid)
{
    std::string eventData = isInKioskMode
                                ? EventFwk::CommonEventSupport::COMMON_EVENT_KIOSK_MODE_ON
                                : EventFwk::CommonEventSupport::COMMON_EVENT_KIOSK_MODE_OFF;
    Want want;
    want.SetAction(eventData);
    want.SetParam("bundleName", bundleName);
    want.SetParam("uid", kioskBundleUid);
    want.SetParam("userId", kioskBundleUid / BASE_USER_RANGE);
    EventFwk::CommonEventData commonData {want};
    if (!IN_PROCESS_CALL(EventFwk::CommonEventManager::PublishCommonEvent(commonData))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PublishCommonEvent failed, eventData: %{public}s", eventData.c_str());
    }
}

bool KioskManager::IsInWhiteListInner(const std::string &bundleName)
{
    return whitelist_.count(bundleName) != 0;
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

bool KioskManager::CheckCallerIsForeground(sptr<IRemoteObject> callerToken)
{
    AppExecFwk::RunningProcessInfo processInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(
        callerToken, processInfo);

    return processInfo.state_ ==
           AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
}
} // namespace AAFwk
} // namespace OHOS
