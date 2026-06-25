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
#include "ffrt.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char* INSIGHT_INTENT_SYS_EVENT_RECERVER = "InsightIntentSysEventReceiver";
const int32_t MAIN_USER_ID = 100;

InsightIntentSysEventReceiver::InsightIntentSysEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
}

bool InsightIntentSysEventReceiver::SaveInsightIntentInfos(const std::string &bundleName, const std::string &moduleName,
    uint32_t versionCode, int32_t userId)
{
    std::vector<std::string> moduleNameVec;
    std::string profile;
    AbilityRuntime::ExtractInsightIntentProfileInfoVec infos = {};
    std::vector<InsightIntentInfo> configIntentInfos = {};
    TAG_LOGI(AAFwkTag::INTENT, "save insight intent infos, bundle:%{public}s module:%{public}s",
        bundleName.c_str(), moduleName.c_str());
    ErrCode ret;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null bundleMgrHelper");
        return false;
    }

    bool anySaved = false;
    OHOS::SplitStr(moduleName, ",", moduleNameVec);
    for (std::string moduleNameLocal : moduleNameVec) {
        // Get json profile firstly
        ret = IN_PROCESS_CALL(bundleMgrHelper->GetJsonProfile(AppExecFwk::INTENT_PROFILE, bundleName,
            moduleNameLocal, profile, userId));
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::INTENT, "GetJsonProfile failed, code: %{public}d", ret);
            if (DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(bundleName)) {
                DeleteInsightIntent(bundleName, moduleNameLocal, userId);
            }
            continue;
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
                bundleName.c_str(), moduleNameLocal.c_str());
            DeleteInsightIntent(bundleName, moduleNameLocal, userId);
            continue;
        }

        // save database
        ret = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
            bundleName, moduleNameLocal, userId, versionCode, infos, configIntentInfos);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "save intent info failed, bundleName: %{public}s, moduleName: %{public}s, "
                "userId: %{public}d", bundleName.c_str(), moduleNameLocal.c_str(), userId);
            continue;
        }
        anySaved = true;

        TAG_LOGI(AAFwkTag::INTENT, "save intent info success, bundleName: %{public}s, moduleName: %{public}s, "
            "userId: %{public}d", bundleName.c_str(), moduleNameLocal.c_str(), userId);
    }
    return anySaved;
}

void InsightIntentSysEventReceiver::RegisterAllFunctions(
    const std::vector<std::pair<std::string, uint32_t>> &newBundles,
    const std::vector<ExtractInsightIntentInfo> &allIntentInfos,
    const std::vector<InsightIntentInfo> &allConfigInfos)
{
    TAG_LOGI(AAFwkTag::INTENT, "register all functions, bundles:%{public}zu intent:%{public}zu config:%{public}zu",
        newBundles.size(), allIntentInfos.size(), allConfigInfos.size());
    std::unordered_map<std::string, std::vector<ExtractInsightIntentInfo>> intentByBundle;
    std::unordered_map<std::string, std::vector<InsightIntentInfo>> configByBundle;
    for (const auto &info : allIntentInfos) {
        intentByBundle[info.genericInfo.bundleName].push_back(info);
    }
    for (const auto &info : allConfigInfos) {
        configByBundle[info.bundleName].push_back(info);
    }
    for (const auto &entry : newBundles) {
        const auto &bundleName = entry.first;
        const auto &intentIt = intentByBundle.find(bundleName);
        const auto &configIt = configByBundle.find(bundleName);
        bool noIntent = intentIt == intentByBundle.end() || intentIt->second.empty();
        bool noConfig = configIt == configByBundle.end() || configIt->second.empty();
        if (noIntent && noConfig) {
            TAG_LOGW(AAFwkTag::INTENT, "register skip empty bundle:%{public}s", bundleName.c_str());
            continue;
        }
        const auto &intents = noIntent ? std::vector<ExtractInsightIntentInfo>{} : intentIt->second;
        const auto &configs = noConfig ? std::vector<InsightIntentInfo>{} : configIt->second;
        CliTool::RegisterInsightIntentFunctions(intents, configs, bundleName, entry.second);
    }
    TAG_LOGI(AAFwkTag::INTENT, "register all functions done");
}

void InsightIntentSysEventReceiver::DeleteInsightIntent(const std::string &bundleName,
    const std::string &moduleName, int32_t userId)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configIntentInfos;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfoByName(
        bundleName, userId, intentInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfoByName(
        bundleName, userId, configIntentInfos);
    if (!intentInfos.empty() || !configIntentInfos.empty()) {
        TAG_LOGI(AAFwkTag::INTENT, "update bundleName: %{public}s to no insight intent",
            bundleName.c_str());
        DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
            bundleName, moduleName, userId);
        CliTool::UnregisterInsightIntentFunctions(bundleName);
    }
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
        DelayedSingleton<InsightIntentDbCache>::GetInstance()->
            GetAllInsightIntentInfo(userId, allIntentInfos, allConfigInfos);
        self->RegisterAllFunctions(newBundles, allIntentInfos, allConfigInfos);
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
        bool rdbHas = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->
            HasInsightIntentByName(bundleInfo.versionCode, bundleInfo.name, userId);
        if (rdbHas) {
            newBundles.emplace_back(bundleInfo.name, bundleInfo.versionCode);
            continue;
        }
        bool anySaved = false;
        for (const auto &hapInfo : bundleInfo.hapModuleInfos) {
            anySaved = SaveInsightIntentInfos(bundleInfo.name, hapInfo.moduleName,
                bundleInfo.versionCode, userId) || anySaved;
        }
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
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
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