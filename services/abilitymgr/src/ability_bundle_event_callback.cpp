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

#include "ability_bundle_event_callback.h"

#include "insight_intent_event_mgr.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "parameters.h"
#ifdef SUPPORT_UPMS
#include "uri_permission_manager_client.h"
#endif // SUPPORT_UPMS

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* KEY_TOKEN = "accessTokenId";
constexpr const char* KEY_UID = "uid";
constexpr const char* KEY_USER_ID = "userId";
constexpr const char* KEY_APP_INDEX = "appIndex";
constexpr const char* OLD_WEB_BUNDLE_NAME = "com.ohos.nweb";
constexpr const char* NEW_WEB_BUNDLE_NAME = "com.ohos.arkwebcore";
constexpr const char* ARKWEB_CORE_PACKAGE_NAME = "persist.arkwebcore.package_name";
constexpr const char* BUNDLE_TYPE = "bundleType";
constexpr const char* IS_RECOVER = "isRecover";
const std::string TYPE = "type";
}
AbilityBundleEventCallback::AbilityBundleEventCallback(
    std::shared_ptr<TaskHandlerWrap> taskHandler, std::shared_ptr<AbilityAutoStartupService> abilityAutoStartupService)
    : taskHandler_(taskHandler), abilityAutoStartupService_(abilityAutoStartupService) {}

void AbilityBundleEventCallback::OnReceiveEvent(const EventFwk::CommonEventData eventData)
{
    // env check
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnReceiveEvent failed, taskHandler is nullptr");
        return;
    }
    const Want& want = eventData.GetWant();
    // action contains the change type of haps.
    std::string action = want.GetAction();
    int32_t installType = want.GetIntParam(TYPE, 0);
    std::string bundleName = want.GetElement().GetBundleName();
    std::string moduleName = want.GetElement().GetModuleName();
    auto tokenId = static_cast<uint32_t>(want.GetIntParam(KEY_TOKEN, 0));
    int uid = want.GetIntParam(KEY_UID, 0);
    auto bundleType = want.GetIntParam(BUNDLE_TYPE, 0);
    int userId = want.GetIntParam(KEY_USER_ID, 0);
    int appIndex = want.GetIntParam(KEY_APP_INDEX, 0);
    // verify data
    if (action.empty() || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnReceiveEvent failed, empty action/bundleName");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnReceiveEvent, action:%{public}s.", action.c_str());
    if (bundleType == static_cast<int32_t>(AppExecFwk::BundleType::APP_PLUGIN)) {
        if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED ||
            action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "plugin action: %{public}s, bundleName: %{public}s",
                action.c_str(), bundleName.c_str());
            HandleUpdatedModuleInfo(bundleName, uid, moduleName, true);
        }
        return;
    }

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        IN_PROCESS_CALL_WITHOUT_RET(DelayedSingleton<AppExecFwk::AppMgrClient>::
            GetInstance()->NotifyUninstallOrUpgradeAppEnd(uid));
        // uninstall bundle
        HandleRemoveUriPermission(tokenId);
        HandleUpdatedModuleInfo(bundleName, uid, moduleName, false);
        if (abilityAutoStartupService_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "OnReceiveEvent failed, abilityAutoStartupService is nullptr");
            return;
        }
        abilityAutoStartupService_->DeleteAutoStartupData(bundleName, tokenId);
        AbilityRuntime::InsightIntentEventMgr::DeleteInsightIntentEvent(want.GetElement(), userId, appIndex);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) {
        // install or uninstall module/bundle
        HandleUpdatedModuleInfo(bundleName, uid, moduleName, false);
        AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(want.GetElement(), userId);
        bool isRecover = want.GetBoolParam(IS_RECOVER, false);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "COMMON_EVENT_PACKAGE_ADDED, isRecover:%{public}d", isRecover);
        if (isRecover) {
            HandleAppUpgradeCompleted(uid, installType);
        }
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
        IN_PROCESS_CALL_WITHOUT_RET(DelayedSingleton<AppExecFwk::AppMgrClient>::
            GetInstance()->NotifyUninstallOrUpgradeAppEnd(uid));
        if (bundleName == NEW_WEB_BUNDLE_NAME || bundleName == OLD_WEB_BUNDLE_NAME ||
            bundleName == system::GetParameter(ARKWEB_CORE_PACKAGE_NAME, "false")) {
            HandleRestartResidentProcessDependedOnWeb();
        }
        HandleUpdatedModuleInfo(bundleName, uid, moduleName, false);
        HandleAppUpgradeCompleted(uid, installType);
        if (abilityAutoStartupService_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "OnReceiveEvent failed, abilityAutoStartupService is nullptr");
            return;
        }
        abilityAutoStartupService_->CheckAutoStartupData(bundleName, uid);
        AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(want.GetElement(), userId);
    }
}

void AbilityBundleEventCallback::HandleRemoveUriPermission(uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "HandleRemoveUriPermission: %{public}i", tokenId);
#ifdef SUPPORT_UPMS
    auto ret = IN_PROCESS_CALL(AAFwk::UriPermissionManagerClient::GetInstance().RevokeAllUriPermissions(tokenId));
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Revoke all uri permissions failed.");
    }
#endif // SUPPORT_UPMS
}

void AbilityBundleEventCallback::HandleUpdatedModuleInfo(const std::string &bundleName, int32_t uid,
    const std::string &moduleName, bool isPlugin)
{
    auto task = [bundleName, uid, moduleName, isPlugin]() {
        AbilityEventUtil::HandleModuleInfoUpdated(bundleName, uid, moduleName, isPlugin);
    };
    taskHandler_->SubmitTask(task);
}

void AbilityBundleEventCallback::HandleAppUpgradeCompleted(int32_t uid, int32_t installType)
{
    wptr<AbilityBundleEventCallback> weakThis = this;
    auto task = [weakThis, uid, installType]() {
        sptr<AbilityBundleEventCallback> sharedThis = weakThis.promote();
        if (sharedThis == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "sharedThis is nullptr.");
            return;
        }

        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr is nullptr.");
            return;
        }
        abilityMgr->AppUpgradeCompleted(uid, installType);
    };
    taskHandler_->SubmitTask(task);
}

void AbilityBundleEventCallback::HandleRestartResidentProcessDependedOnWeb()
{
    auto task = []() {
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr is nullptr.");
            return;
        }
        abilityMgr->HandleRestartResidentProcessDependedOnWeb();
    };
    taskHandler_->SubmitTask(task);
}
} // namespace AAFwk
} // namespace OHOS
