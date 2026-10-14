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

#include "insight_intent_event_mgr.h"

#include <unordered_map>

#include "insight_intent_sys_event_receiver.h"
#include "insight_intent_db_cache.h"
#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "function_call_convert.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "extract_insight_intent_profile.h"
#include "os_account_manager_wrapper.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "app_utils.h"
#include "ffrt.h"
#include "global_constant.h"

namespace OHOS {
namespace AbilityRuntime {

InsightIntentEventMgr::DualModeInfo InsightIntentEventMgr::ResolveDualModeInfo(
    const std::string &bundleName, int32_t userId, int32_t appIndexFromEvent)
{
    InsightIntentEventMgr::DualModeInfo dualInfo;
    dualInfo.appIndex = appIndexFromEvent;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "bundleMgrHelper null, fallback same-name-same-package");
        return dualInfo;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName,
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
        bundleInfo, userId));
    if (ret == ERR_OK) {
        auto policy = bundleInfo.deviceModeDistributionPolicy;
        bool isDifferentPackage =
            (policy == AppExecFwk::DeviceModeDistributionPolicy::UNIVERSAL_DIFFERENT_PACKAGE ||
             policy == AppExecFwk::DeviceModeDistributionPolicy::PARTIAL_COMPATIBLE_DIFFERENT_PACKAGE ||
             policy == AppExecFwk::DeviceModeDistributionPolicy::FULL_COMPATIBLE_DIFFERENT_PACKAGE);
        if (isDifferentPackage) {
            dualInfo.isDualMode = true;
            int32_t preferredAppIndex = 0;
            auto dualRet = IN_PROCESS_CALL(bundleMgrHelper->GetDualModeBundleInfo(
                bundleName, userId, preferredAppIndex));
            if (dualRet == ERR_OK) {
                dualInfo.appIndex = preferredAppIndex;
            } else {
                dualInfo.appIndex = -1;
            }
            TAG_LOGI(AAFwkTag::INTENT, "same-name-different-package app, bundle:%{public}s, "
                "policy:%{public}d, preferredAppIndex:%{public}d",
                bundleName.c_str(), static_cast<int32_t>(policy), dualInfo.appIndex);
        } else {
            dualInfo.isDualMode = false;
            dualInfo.appIndex = 0;
            TAG_LOGI(AAFwkTag::INTENT, "same-name-same-package app, bundle:%{public}s, policy:%{public}d",
                bundleName.c_str(), static_cast<int32_t>(policy));
        }
    } else {
        TAG_LOGI(AAFwkTag::INTENT, "GetBundleInfoV9 failed, bundle:%{public}s, fallback by appIndex",
            bundleName.c_str());
    }
    return dualInfo;
}

void InsightIntentEventMgr::DeleteInsightIntent(const std::string &bundleName, const std::string &moduleName,
    int32_t userId, const DualModeInfo &dualInfo)
{
    if (!DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId, dualInfo.appIndex, dualInfo.isDualMode)) {
        return;
    }
    TAG_LOGI(AAFwkTag::INTENT, "update bundleName: %{public}s to no insight intent", bundleName.c_str());
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentEventMgr::UpdateInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId,
    int32_t appIndex)
{
    auto bundleName = elementName.GetBundleName();
    auto moduleName = elementName.GetModuleNameRef();
    if (bundleName.empty() || moduleName.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "input param empty, bundleName: %{public}s", bundleName.c_str());
        return;
    }
    if (userId < 0) {
        TAG_LOGW(AAFwkTag::INTENT, "invalid userId: %{public}d", userId);
        return;
    }
    if (AAFwk::AppUtils::GetInstance().IsBopdOrRescueMode()) {
        TAG_LOGI(AAFwkTag::INTENT, "BOPD or rescue mode, skip package update: %{public}s", bundleName.c_str());
        return;
    }
    auto dualInfo = ResolveDualModeInfo(bundleName, userId, appIndex);
    // 更新场景使用事件 appIndex（正在更新的模式），而非偏好 appIndex（查询用的当前模式）
    dualInfo.appIndex = appIndex;
    ffrt::submit([moduleName, bundleName, userId, dualInfo, versionCode = 0] () mutable {
        auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
        if (bundleMgrHelper == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "get bundleMgrHelper instance failed");
            return;
        }
        AppExecFwk::BundleInfo bundleInfo;
        if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName,
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
            bundleInfo, userId)) != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "get bundle info failed");
            return;
        }
        versionCode = bundleInfo.versionCode;
        ModuleCollectResult result;
        CollectModuleSaveParams(bundleName, moduleName, userId, dualInfo, result);
        if (result.allInfos.insightIntents.empty() && result.allConfigInfos.empty()) {
            ClearStaleIntentFunctions(bundleName, userId);
            return;
        }
        for (auto &sp : result.saveParams) {
            sp.bundleName = bundleName;
            sp.userId = userId;
            sp.versionCode = versionCode;
            sp.appIndex = dualInfo.appIndex;
            sp.isDualMode = dualInfo.isDualMode;
        }
        DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->SaveBatchInsightIntentTotalInfo(
            result.saveParams);
        FilterAndRegisterIntentFunctions(bundleName, userId, versionCode,
            result.allInfos, result.allConfigInfos);
    });
}

void InsightIntentEventMgr::CollectModuleSaveParams(const std::string &bundleName,
    const std::string &moduleName, int32_t userId, const DualModeInfo &dualInfo,
    ModuleCollectResult &result)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get bundleMgrHelper instance failed");
        return;
    }
    std::vector<std::string> moduleNameVec;
    OHOS::SplitStr(moduleName, ",", moduleNameVec);
    for (std::string moduleNameLocal : moduleNameVec) {
        std::string profile;
        ErrCode ret = IN_PROCESS_CALL(bundleMgrHelper->GetJsonProfile(AppExecFwk::INTENT_PROFILE,
            bundleName, moduleNameLocal, profile, userId));
        if (ret != ERR_OK) {
            TAG_LOGI(AAFwkTag::INTENT, "get json failed code: %{public}d", ret);
            DeleteInsightIntent(bundleName, moduleNameLocal, userId, dualInfo);
            continue;
        }
        ExtractInsightIntentProfileInfoVec infos = {};
        std::vector<InsightIntentInfo> configIntentInfos = {};
        bool isTransformExtractIntent = (!AbilityRuntime::ExtractInsightIntentProfile::TransformTo(
            profile, infos) || infos.insightIntents.size() == 0);
        bool isTransformConfigIntent = (!AbilityRuntime::InsightIntentProfile::TransformTo(
            profile, configIntentInfos) || configIntentInfos.size() == 0);
        if (isTransformExtractIntent && isTransformConfigIntent) {
            TAG_LOGW(AAFwkTag::INTENT, "transform profile failed, profile:%{public}s", profile.c_str());
            DeleteInsightIntent(bundleName, moduleNameLocal, userId, dualInfo);
            continue;
        }
        for (auto &item : configIntentInfos) {
            item.bundleName = bundleName;
            item.moduleName = moduleNameLocal;
        }
        for (auto &item : infos.insightIntents) {
            item.bundleName = bundleName;
            item.moduleName = moduleNameLocal;
        }
        InsightIntentSaveParam saveParam;
        saveParam.moduleName = moduleNameLocal;
        saveParam.profileInfos = infos;
        saveParam.configInfos = configIntentInfos;
        result.saveParams.emplace_back(std::move(saveParam));
        for (const auto &item : infos.insightIntents) {
            result.allInfos.insightIntents.push_back(item);
        }
        for (const auto &item : configIntentInfos) {
            result.allConfigInfos.push_back(item);
        }
    }
}

void InsightIntentEventMgr::ClearStaleIntentFunctions(const std::string &bundleName, int32_t userId)
{
    TAG_LOGI(AAFwkTag::INTENT, "no intent in new version, clear stale functions, bundle:%{public}s",
        bundleName.c_str());
    bool unregistered = CliTool::UnregisterInsightIntentFunctions(bundleName, userId);
    TAG_LOGI(AAFwkTag::INTENT, "unregister done, bundle:%{public}s, success:%{public}d",
        bundleName.c_str(), unregistered);
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentEventMgr::FilterAndRegisterIntentFunctions(const std::string &bundleName,
    int32_t userId, uint32_t versionCode,
    const ExtractInsightIntentProfileInfoVec &allInfos,
    std::vector<InsightIntentInfo> &allConfigInfos)
{
    TAG_LOGI(AAFwkTag::INTENT, "collected intents for batch update, profile:%{public}zu config:%{public}zu, "
        "bundle:%{public}s", allInfos.insightIntents.size(), allConfigInfos.size(), bundleName.c_str());
    std::vector<AbilityRuntime::ExtractInsightIntentInfo> genericInfos;
    for (const auto &profileInfo : allInfos.insightIntents) {
        AbilityRuntime::ExtractInsightIntentInfo generic;
        if (AbilityRuntime::ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, generic)) {
            genericInfos.push_back(std::move(generic));
        }
    }
    CliTool::IntentFilterUtil intentFilter;
    TAG_LOGI(AAFwkTag::INTENT, "before filter, generic:%{public}zu config:%{public}zu",
        genericInfos.size(), allConfigInfos.size());
    intentFilter.FilterGeneric(genericInfos);
    intentFilter.FilterConfig(allConfigInfos);
    TAG_LOGI(AAFwkTag::INTENT, "after filter, generic:%{public}zu config:%{public}zu, bundle:%{public}s",
        genericInfos.size(), allConfigInfos.size(), bundleName.c_str());
    CliTool::BatchUpdateInsightIntentFunctions(
        genericInfos, allConfigInfos, bundleName, versionCode, userId);
    TAG_LOGI(AAFwkTag::INTENT, "batch update request done, bundle:%{public}s", bundleName.c_str());
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentEventMgr::DeleteInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId,
    int32_t appIndex)
{
    auto bundleName = elementName.GetBundleName();
    auto moduleName = elementName.GetModuleNameRef();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "input bundleName empty, bundleName: %{public}s, moduleName: %{public}s",
            bundleName.c_str(), moduleName.c_str());
        return;
    }

    // 克隆/DLP/AI Agent 索引：维持原有跳过行为
    if (appIndex > 0 && appIndex != AbilityRuntime::GlobalConstant::PC_TABLET_INDEX) {
        TAG_LOGI(AAFwkTag::INTENT, "clone/dlp app, skip delete, "
            "bundleName: %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
        return;
    }

    if (userId < 0) {
        TAG_LOGI(AAFwkTag::INTENT, "invalid userId: %{public}d", userId);
        return;
    }

    if (AAFwk::AppUtils::GetInstance().IsBopdOrRescueMode()) {
        TAG_LOGI(AAFwkTag::INTENT, "BOPD or rescue mode, skip package delete: %{public}s", bundleName.c_str());
        return;
    }

    // 卸载场景不能依赖 GetBundleInfoV9（应用已卸载），使用 DeleteIntentByAppIndex 按 Key 段数
    // 精确删除 RDB：只删目标模式，不影响其他模式；随后同步清理 bundleVersionMap_ 内存缓存
    ffrt::submit([bundleName, moduleName, userId, appIndex]() {
        auto storage = DelayedSingleton<AbilityRuntime::InsightRdbStorageMgr>::GetInstance();
        storage->DeleteIntentByAppIndex(bundleName, moduleName, userId, appIndex);
        bool isDualMode = (appIndex == AbilityRuntime::GlobalConstant::PC_TABLET_INDEX);
        auto dbCache = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance();
        dbCache->RemoveBundleVersionCacheEntry(bundleName, userId, appIndex, isDualMode, moduleName);
        dbCache->BackupRdb();
        bool unregistered = CliTool::UnregisterInsightIntentFunctions(bundleName, userId);
        TAG_LOGI(AAFwkTag::INTENT, "delete done, bundleName: %{public}s, moduleName: %{public}s, "
            "userId: %{public}d, appIndex: %{public}d, unregistered: %{public}d",
            bundleName.c_str(), moduleName.c_str(), userId, appIndex, unregistered);
    });
}

void InsightIntentEventMgr::SubscribeSysEventReceiver()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    insightIntentSysEventReceiver_ = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);
    bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(insightIntentSysEventReceiver_);
    if (!subResult) {
        TAG_LOGE(AAFwkTag::INTENT, "subscribe common event failed");
        return;
    }

    TAG_LOGI(AAFwkTag::INTENT, "subscribe common event success");
}

} // namespace AbilityRuntime
} // namespace OHOS
