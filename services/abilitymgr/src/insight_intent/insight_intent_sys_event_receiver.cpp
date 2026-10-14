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

#include "insight_intent_sys_event_receiver.h"

#include <unordered_map>

#include "extract_insight_intent_profile.h"
#include "common_event_support.h"
#include "function_call_convert.h"
#include "hilog_tag_wrapper.h"
#include "task_handler_wrap.h"
#include "os_account_manager_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ability_util.h"
#include "bundle_mgr_helper.h"
#include "insight_intent_db_cache.h"
#include "app_utils.h"
#include "ffrt.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char* INSIGHT_INTENT_SYS_EVENT_RECERVER = "InsightIntentSysEventReceiver";
const int32_t MAIN_USER_ID = 100;

InsightIntentSysEventReceiver::InsightIntentSysEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
}

bool InsightIntentSysEventReceiver::SaveInsightIntentInfos(const std::string &bundleName,
    const std::vector<std::string> &moduleNames, const InsightIntentSaveParam &templateParam)
{
    TAG_LOGI(AAFwkTag::INTENT, "save insight intent infos, bundle:%{public}s moduleCount:%{public}zu",
        bundleName.c_str(), moduleNames.size());
    std::vector<InsightIntentSaveParam> saveParams;
    for (const auto &moduleNameLocal : moduleNames) {
        InsightIntentSaveParam saveParam = templateParam;
        saveParam.bundleName = bundleName;
        saveParam.moduleName = moduleNameLocal;
        if (CollectInsightIntentSaveParam(bundleName, moduleNameLocal, saveParam)) {
            saveParams.emplace_back(std::move(saveParam));
        }
    }
    ErrCode ret = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->
        SaveBatchInsightIntentTotalInfo(saveParams);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "save intent infos failed, bundleName: %{public}s, userId: %{public}d, "
            "ret: %{public}d", bundleName.c_str(), templateParam.userId, ret);
        return false;
    }
    if (!saveParams.empty()) {
        TAG_LOGI(AAFwkTag::INTENT, "save intent infos success, bundleName: %{public}s, moduleCount: %{public}zu, "
            "userId: %{public}d", bundleName.c_str(), moduleNames.size(), templateParam.userId);
    }
    return !saveParams.empty();
}

bool InsightIntentSysEventReceiver::CollectInsightIntentSaveParam(const std::string &bundleName,
    const std::string &moduleName, InsightIntentSaveParam &saveParam)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null bundleMgrHelper");
        return false;
    }
    // Get json profile firstly
    std::string profile;
    AbilityRuntime::ExtractInsightIntentProfileInfoVec infos;
    std::vector<InsightIntentInfo> configIntentInfos;
    ErrCode ret = IN_PROCESS_CALL(bundleMgrHelper->GetJsonProfile(AppExecFwk::INTENT_PROFILE, bundleName,
        moduleName, profile, saveParam.userId));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetJsonProfile failed, code: %{public}d", ret);
        DeleteInsightIntent(bundleName, moduleName, saveParam.userId, saveParam.appIndex, saveParam.isDualMode);
        return false;
    }
    // Transform json string
    bool isTransformExtractIntent = (!AbilityRuntime::ExtractInsightIntentProfile::TransformTo(profile, infos) ||
        infos.insightIntents.size() == 0);
    bool isTransformConfigIntent = (
        !AbilityRuntime::InsightIntentProfile::TransformTo(profile, configIntentInfos) ||
        configIntentInfos.size() == 0);
    if (isTransformExtractIntent && isTransformConfigIntent) {
        TAG_LOGW(AAFwkTag::INTENT,
            "transform profile failed, deleting config, bundle:%{public}s module:%{public}s",
            bundleName.c_str(), moduleName.c_str());
        DeleteInsightIntent(bundleName, moduleName, saveParam.userId, saveParam.appIndex, saveParam.isDualMode);
        return false;
    }
    // force BMS bundleName/moduleName over the profile values so the function
    // namespace matches the name UnregisterInsightIntentFunctions uses and the
    // db cache erase-by-moduleName hits stale entries
    for (auto &item : configIntentInfos) {
        item.bundleName = bundleName;
        item.moduleName = moduleName;
    }
    for (auto &item : infos.insightIntents) {
        item.bundleName = bundleName;
        item.moduleName = moduleName;
    }
    saveParam.bundleName = bundleName;
    saveParam.moduleName = moduleName;
    saveParam.profileInfos = infos;
    saveParam.configInfos = configIntentInfos;
    return true;
}

void InsightIntentSysEventReceiver::RegisterAllFunctions(
    const std::vector<std::pair<std::string, uint32_t>> &newBundles,
    const std::vector<ExtractInsightIntentInfo> &allIntentInfos,
    const std::vector<InsightIntentInfo> &allConfigInfos, int32_t userId)
{
    TAG_LOGI(AAFwkTag::INTENT, "register all functions, bundles:%{public}zu intent:%{public}zu config:%{public}zu",
        newBundles.size(), allIntentInfos.size(), allConfigInfos.size());
    if (allIntentInfos.empty() && allConfigInfos.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "register skip: no intent data");
        return;
    }
    std::unordered_map<std::string, uint32_t> bundleVersionMap;
    for (const auto &entry : newBundles) {
        bundleVersionMap[entry.first] = entry.second;
    }
    CliTool::BatchRegisterInsightIntentFunctions(
        allIntentInfos, allConfigInfos, bundleVersionMap, userId);
    TAG_LOGI(AAFwkTag::INTENT, "register all functions request done");
}

void InsightIntentSysEventReceiver::DeleteInsightIntent(const std::string &bundleName,
    const std::string &moduleName, int32_t userId, int32_t appIndex, bool isDualMode)
{
    if (!DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId, appIndex, isDualMode)) {
        return;
    }
    TAG_LOGI(AAFwkTag::INTENT, "update bundleName: %{public}s to no insight intent", bundleName.c_str());
    CliTool::UnregisterInsightIntentFunctions(bundleName, userId);
}

int32_t InsightIntentSysEventReceiver::ResolveLoadUserId(int32_t userId)
{
    if (userId != -1) {
        return userId;
    }
    int32_t current = AppExecFwk::OsAccountManagerWrapper::GetCurrentActiveAccountId();
    if (current == 0) {
        TAG_LOGI(AAFwkTag::INTENT, "use MAIN_USER_ID(%{public}d) instead of current userId: (%{public}d)",
            MAIN_USER_ID, current);
        return MAIN_USER_ID;
    }
    return current;
}

void InsightIntentSysEventReceiver::BackupAndScheduleRegister(
    std::vector<std::pair<std::string, uint32_t>> &&newBundles, int32_t userId)
{
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
    auto self = shared_from_this();
    auto task = [self, newBundles = std::move(newBundles), userId]() {
        std::vector<ExtractInsightIntentInfo> allIntentInfos;
        std::vector<InsightIntentInfo> allConfigInfos;
        DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllInsightIntentInfoForRegister(
            userId, allIntentInfos, allConfigInfos);
        TAG_LOGI(AAFwkTag::INTENT, "GetAllForRegister done, intent:%{public}zu config:%{public}zu bundles:%{public}zu",
            allIntentInfos.size(), allConfigInfos.size(), newBundles.size());
        self->RegisterAllFunctions(newBundles, allIntentInfos, allConfigInfos, userId);
    };
    ffrt::submit(task);
}

void InsightIntentSysEventReceiver::LoadInsightIntentInfos(int32_t userId)
{
    userId = ResolveLoadUserId(userId);
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null bundleMgrHelper");
        return;
    }

    std::vector<AppExecFwk::BundleInfo> bundleInfos {};
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfosV9(
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
        bundleInfos, userId)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get bundle info failed");
        return;
    }

    std::vector<std::pair<std::string, uint32_t>> newBundles;
    for (auto &bundleInfo : bundleInfos) {
        auto policy = bundleInfo.deviceModeDistributionPolicy;
        bool isDifferentPackage =
            (policy == AppExecFwk::DeviceModeDistributionPolicy::UNIVERSAL_DIFFERENT_PACKAGE ||
             policy == AppExecFwk::DeviceModeDistributionPolicy::PARTIAL_COMPATIBLE_DIFFERENT_PACKAGE ||
             policy == AppExecFwk::DeviceModeDistributionPolicy::FULL_COMPATIBLE_DIFFERENT_PACKAGE);
        int32_t appIndex = isDifferentPackage ? bundleInfo.appIndex : 0;
        bool rdbHas = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->
            HasInsightIntentByName(bundleInfo.versionCode, bundleInfo.name, userId, appIndex, isDifferentPackage);
        if (rdbHas) {
            newBundles.emplace_back(bundleInfo.name, bundleInfo.versionCode);
            continue;
        }
        std::vector<std::string> moduleNames;
        for (const auto &hapInfo : bundleInfo.hapModuleInfos) {
            moduleNames.push_back(hapInfo.moduleName);
        }
        bool anySaved = SaveInsightIntentInfos(bundleInfo.name, moduleNames,
            AbilityRuntime::InsightIntentSaveParam{"", "", bundleInfo.versionCode, userId,
                {}, {}, appIndex, isDifferentPackage});
        if (anySaved) {
            newBundles.emplace_back(bundleInfo.name, bundleInfo.versionCode);
        }
    }
    if (!newBundles.empty()) {
        BackupAndScheduleRegister(std::move(newBundles), userId);
    }
}

void InsightIntentSysEventReceiver::DeleteInsightIntentInfoByUserId(int32_t userId)
{
    int32_t ret = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->DeleteInsightIntentByUserId(
        userId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "delete insight intent by userId failed, userId: %{public}d, ret: %{public}d",
            userId, ret);
        return;
    }
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentSysEventReceiver::HandleBundleScanFinished()
{
    auto task = [self = shared_from_this()]() {
        self->LoadInsightIntentInfos();
    };
    ffrt::submit(task);
}

void InsightIntentSysEventReceiver::HandleUserSwitched(const EventFwk::CommonEventData &data)
{
    int32_t userId = data.GetCode();
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid switched userId: %{public}d", userId);
    }

    std::lock_guard<std::mutex> lock(userIdMutex_);
    if (userId == lastUserId_) {
        TAG_LOGE(AAFwkTag::INTENT, "same userId: %{public}d", lastUserId_);
        return;
    }

    TAG_LOGI(AAFwkTag::INTENT, "userId: %{public}d switch to  current userId: %{public}d", lastUserId_, userId);
    lastUserId_ = userId;

    if (DelayedSingleton<InsightIntentDbCache>::GetInstance()->IsCacheInitialized(userId)) {
        TAG_LOGI(AAFwkTag::INTENT, "UserSwitched: cache already initialized for userId: %{public}d, skipped",
            userId);
        return;
    }

    auto task = [self = shared_from_this(), userId]() {
        self->LoadInsightIntentInfos(userId);
    };
    ffrt::submit(task);
}

void InsightIntentSysEventReceiver::HandleUserRemove(const EventFwk::CommonEventData &data)
{
    int32_t userId = data.GetCode();
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid switched userId: %{public}d", userId);
    }

    std::lock_guard<std::mutex> lock(userIdMutex_);
    if (userId == lastUserId_) {
        TAG_LOGW(AAFwkTag::INTENT, "not allow remove current userId: %{public}d", userId);
        return;
    }

    auto task = [self = shared_from_this(), userId]() { self->DeleteInsightIntentInfoByUserId(userId); };
    ffrt::submit(task);
}

void InsightIntentSysEventReceiver::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    if (AAFwk::AppUtils::GetInstance().IsBopdOrRescueMode()) {
        TAG_LOGI(AAFwkTag::INTENT, "BOPD or rescue mode, skip sys event: %{public}s",
            data.GetWant().GetAction().c_str());
        return;
    }
    const AAFwk::Want &want = data.GetWant();
    const auto &action = want.GetActionRef();
    TAG_LOGI(AAFwkTag::INTENT, "the action: %{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED) {
        HandleBundleScanFinished();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        HandleUserSwitched(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        HandleUserRemove(data);
    } else {
        TAG_LOGW(AAFwkTag::INTENT, "invalid action");
    }
}
} // namespace AbilityRuntime
} // namespace OHOS