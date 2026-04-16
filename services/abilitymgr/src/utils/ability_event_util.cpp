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

#include "ability_event_util.h"
#include "ability_bundle_manager_helper/bundle_mgr_helper.h"
#include "app_scheduler.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_publish_info.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t FFRT_TASK_TIMEOUT = 5 * 1000 * 1000;  // 5s

void AbilityEventUtil::HandleModuleInfoUpdated(const std::string &bundleName, const int uid,
    const std::string& moduleName, bool isPlugin)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "HandleModuleInfoUpdated start.");
    DelayedSingleton<AppScheduler>::GetInstance()->UpdateApplicationInfoInstalled(bundleName, uid, moduleName,
        isPlugin);
}

void AbilityEventUtil::SendStartAbilityErrorEvent(EventInfo &eventInfo, int32_t errCode, const std::string errMsg,
    bool isSystemError)
{
    if (errCode == ERR_OK) {
        return;
    }
    EventName name = isSystemError ? EventName::START_ABILITY_SYSTEM_ERROR : EventName::START_ABILITY_ERROR;
    eventInfo.errCode = errCode;
    eventInfo.errMsg = errMsg;
    ffrt::submit([name, eventInfo]() {
        EventReport::SendAbilityEvent(name, HISYSEVENT_FAULT, eventInfo);
        }, ffrt::task_attr().timeout(FFRT_TASK_TIMEOUT));
}

void AbilityEventUtil::SendKillProcessWithReasonEvent(int32_t errCode, const std::string &errMsg, EventInfo &eventInfo)
{
    EventName name = EventName::KILL_PROCESS_WITH_REASON;
    eventInfo.errCode = errCode;
    eventInfo.errMsg = errMsg;
    ffrt::submit([name, eventInfo]() {
        EventReport::SendAbilityEvent(name, HISYSEVENT_STATISTIC, eventInfo);
        }, ffrt::task_attr().timeout(FFRT_TASK_TIMEOUT));
}

bool AbilityEventUtil::HandleBundleFirstLaunch(const AppExecFwk::ApplicationInfo &appInfo, int32_t userId,
    const std::string &callerBundleName)
{
    if (appInfo.isBundleFirstLaunched) {
        return true;  // Already launched, no need to process
    }

    // Get appId from SignatureInfo
    std::string appId;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper != nullptr) {
        AppExecFwk::SignatureInfo signatureInfo;
        if (IN_PROCESS_CALL(bundleMgrHelper->GetSignatureInfoByBundleName(
            appInfo.bundleName, signatureInfo)) == ERR_OK) {
            appId = signatureInfo.appId;
        }
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "HandleBundleFirstLaunch : bundleName = %{public}s, userId = %{public}d, "
        "appIndex = %{public}d, appId = %{public}s, callerBundleName = %{public}s",
        appInfo.bundleName.c_str(), userId, appInfo.appIndex, appId.c_str(), callerBundleName.c_str());

    // Re-check isBundleFirstLaunched by querying from bundle manager (posttask async execution)
    AppExecFwk::ApplicationInfo latestAppInfo;
    if (bundleMgrHelper != nullptr) {
        if (IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfoWithAppIndex(
            appInfo.bundleName, appInfo.appIndex, userId, latestAppInfo)) && latestAppInfo.isBundleFirstLaunched) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Bundle already launched, skipping event: bundleName=%{public}s",
                appInfo.bundleName.c_str());
            return true;
        }
    }

    // Publish COMMON_EVENT_APP_FIRST_LAUNCH event
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_APP_FIRST_LAUNCH);
    want.SetParam("bundleName", appInfo.bundleName);
    want.SetParam("userId", userId);
    want.SetParam("appIndex", appInfo.appIndex);
    want.SetParam("uid", appInfo.uid);
    if (!appId.empty()) {
        want.SetParam("appId", appId);
    }
    want.SetParam("callerBundleName", callerBundleName);
    EventFwk::CommonEventData commonData {want};

    // Set subscriber permissions to ohos.permission.INSTALL_BUNDLE
    EventFwk::CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> permissions;
    permissions.emplace_back("ohos.permission.INSTALL_BUNDLE");
    commonEventPublishInfo.SetSubscriberPermissions(permissions);

    if (!IN_PROCESS_CALL(EventFwk::CommonEventManager::PublishCommonEvent(commonData, commonEventPublishInfo))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PublishCommonEvent COMMON_EVENT_APP_FIRST_LAUNCH failed");
        return false;
    }

    // Set bundle first launch status to true after first launch
    if (bundleMgrHelper != nullptr) {
        auto ret = IN_PROCESS_CALL(bundleMgrHelper->SetBundleFirstLaunch(
            appInfo.bundleName, userId, appInfo.appIndex, true));
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "SetBundleFirstLaunch failed: bundleName=%{public}s, err=%{public}d",
                appInfo.bundleName.c_str(), ret);
        }
    }

    return true;
}

} // namespace AAFwk
} // namespace OHOS
