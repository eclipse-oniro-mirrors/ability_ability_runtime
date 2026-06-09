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

#include "insight_intent_matcher.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_db_cache.h"
#include "user_controller.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t DEFAULT_INVAL_VALUE = -1;

int32_t GetValidUserId(int32_t userId)
{
    return DEFAULT_INVAL_VALUE == userId ? UserController::GetInstance().GetCallerUserId() : userId;
}
} // namespace

int32_t InsightIntentMatcher::GetMatchedIntentInfo(const std::string &bundleName,
    const std::string &intentName, int32_t userId, ExtractInsightIntentGenericInfo &matchedInfo)
{
    std::vector<ExtractInsightIntentGenericInfo> allInfos;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentGenericInfoByName(
        bundleName, GetValidUserId(userId), allInfos);
    for (const auto &info : allInfos) {
        if (info.intentName == intentName) {
            matchedInfo = info;
            return ERR_OK;
        }
    }

    std::vector<InsightIntentInfo> configInfos;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfoByName(
        bundleName, GetValidUserId(userId), configInfos);
    for (const auto &config : configInfos) {
        if (config.intentName == intentName) {
            ConvertConfigToGenericInfo(config, matchedInfo);
            return ERR_OK;
        }
    }

    TAG_LOGE(AAFwkTag::INTENT, "intent not found: %{public}s/%{public}s",
        bundleName.c_str(), intentName.c_str());
    return ERR_INVALID_VALUE;
}

void InsightIntentMatcher::ConvertConfigToGenericInfo(const InsightIntentInfo &config,
    ExtractInsightIntentGenericInfo &generic)
{
    generic.bundleName = config.bundleName;
    generic.moduleName = config.moduleName;
    generic.intentName = config.intentName;
    generic.decoratorType = "";
    if (!config.uiAbilityIntentInfo.abilityName.empty()) {
        generic.set<InsightIntentEntryInfo>();
        auto &entry = generic.get<InsightIntentEntryInfo>();
        entry.abilityName = config.uiAbilityIntentInfo.abilityName;
        entry.executeMode = config.uiAbilityIntentInfo.supportExecuteMode;
    } else if (!config.serviceExtensionIntentInfo.abilityName.empty()) {
        generic.set<InsightIntentEntryInfo>();
        auto &entry = generic.get<InsightIntentEntryInfo>();
        entry.abilityName = config.serviceExtensionIntentInfo.abilityName;
        entry.executeMode = {AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY};
    } else if (!config.uiExtensionIntentInfo.abilityName.empty()) {
        generic.set<InsightIntentEntryInfo>();
        auto &entry = generic.get<InsightIntentEntryInfo>();
        entry.abilityName = config.uiExtensionIntentInfo.abilityName;
        entry.executeMode = {AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY};
    } else if (!config.formIntentInfo.abilityName.empty()) {
        generic.set<InsightIntentFormInfo>();
        auto &form = generic.get<InsightIntentFormInfo>();
        form.abilityName = config.formIntentInfo.abilityName;
        form.formName = config.formIntentInfo.formName;
    }
}

int32_t InsightIntentMatcher::ParseIntentExecuteMode(const ExtractInsightIntentGenericInfo &matchedInfo,
    std::string &abilityName, int32_t &executeMode)
{
    executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    if (matchedInfo.currentType == InfoType::Entry) {
        auto &entryInfo = matchedInfo.get<InsightIntentEntryInfo>();
        abilityName = entryInfo.abilityName;
        if (!entryInfo.executeMode.empty()) {
            executeMode = static_cast<int32_t>(entryInfo.executeMode.front());
        }
    } else if (matchedInfo.currentType == InfoType::Function) {
        executeMode = static_cast<int32_t>(AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY);
    } else if (matchedInfo.currentType == InfoType::Page) {
        auto &pageInfo = matchedInfo.get<InsightIntentPageInfo>();
        abilityName = pageInfo.uiAbility;
    } else if (matchedInfo.currentType == InfoType::Form) {
        auto &formInfo = matchedInfo.get<InsightIntentFormInfo>();
        abilityName = formInfo.abilityName;
    } else if (matchedInfo.currentType == InfoType::Link) {
        abilityName = "";
    }
    if (abilityName.empty() && matchedInfo.currentType != InfoType::Link &&
        matchedInfo.currentType != InfoType::Function) {
        TAG_LOGE(AAFwkTag::INTENT, "abilityName empty");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
