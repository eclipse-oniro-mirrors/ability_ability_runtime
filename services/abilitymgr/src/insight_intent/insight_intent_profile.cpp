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

#include "insight_intent_profile.h"

#include "hilog_tag_wrapper.h"
#include "insight_intent_json_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
int32_t g_parseResult = ERR_OK;
std::mutex g_mutex;

const std::string INSIGHT_INTENTS = "insightIntents";
const std::string INSIGHT_INTENT_NAME = "intentName";
const std::string INSIGHT_INTENT_DOMAIN = "domain";
const std::string INSIGHT_INTENT_VERSION = "intentVersion";
const std::string INSIGHT_INTENT_SRC_ENTRY = "srcEntry";
const std::string INSIGHT_INTENT_UI_ABILITY = "uiAbility";
const std::string INSIGHT_INTENT_UI_EXTENSION = "uiExtension";
const std::string INSIGHT_INTENT_SERVICE_EXTENSION = "serviceExtension";
const std::string INSIGHT_INTENT_FORM = "form";
const std::string INSIGHT_INTENT_ABILITY = "ability";
const std::string INSIGHT_INTENT_EXECUTE_MODE = "executeMode";
const std::string INSIGHT_INTENT_FORM_NAME = "formName";

const std::map<std::string, ExecuteMode> executeModeMap = {
    {"foreground", ExecuteMode::UI_ABILITY_FOREGROUND},
    {"background", ExecuteMode::UI_ABILITY_BACKGROUND}
};

struct UIAbilityProfileInfo {
    std::string abilityName;
    std::vector<std::string> supportExecuteMode {};
};

struct UIExtensionProfileInfo {
    std::string abilityName;
};

struct ServiceExtensionProfileInfo {
    std::string abilityName;
};

struct FormProfileInfo {
    std::string abilityName;
    std::string formName;
};

struct InsightIntentProfileInfo {
    std::string intentName;
    std::string intentDomain;
    std::string intentVersion;
    std::string srcEntry;
    UIAbilityProfileInfo uiAbilityProfileInfo;
    UIExtensionProfileInfo uiExtensionProfileInfo;
    ServiceExtensionProfileInfo serviceExtensionProfileInfo;
    FormProfileInfo formProfileInfo;
};

struct InsightIntentProfileInfoVec {
    std::vector<InsightIntentProfileInfo> insightIntents {};
};

void from_json(const cJSON *jsonObject, UIAbilityProfileInfo &info)
{
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_ABILITY, info.abilityName, true, g_parseResult);
    GetStringValuesIfFindKey(jsonObject, INSIGHT_INTENT_EXECUTE_MODE, info.supportExecuteMode, true, g_parseResult);
}

void from_json(const cJSON *jsonObject, UIExtensionProfileInfo &info)
{
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_ABILITY, info.abilityName, true, g_parseResult);
}

void from_json(const cJSON *jsonObject, ServiceExtensionProfileInfo &info)
{
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_ABILITY, info.abilityName, true, g_parseResult);
}

void from_json(const cJSON *jsonObject, FormProfileInfo &info)
{
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_ABILITY, info.abilityName, true, g_parseResult);
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_FORM_NAME, info.formName, true, g_parseResult);
}

void from_json(const cJSON *jsonObject, InsightIntentProfileInfo &insightIntentInfo)
{
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_NAME, insightIntentInfo.intentName, true, g_parseResult);
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_DOMAIN, insightIntentInfo.intentDomain, true, g_parseResult);
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_VERSION, insightIntentInfo.intentVersion, true, g_parseResult);
    GetStringValueIfFindKey(jsonObject, INSIGHT_INTENT_SRC_ENTRY, insightIntentInfo.srcEntry, true, g_parseResult);
    GetObjectValueIfFindKey(jsonObject, INSIGHT_INTENT_UI_ABILITY, insightIntentInfo.uiAbilityProfileInfo, false,
        g_parseResult);
    GetObjectValueIfFindKey(jsonObject, INSIGHT_INTENT_UI_EXTENSION, insightIntentInfo.uiExtensionProfileInfo, false,
        g_parseResult);
    GetObjectValueIfFindKey(jsonObject, INSIGHT_INTENT_SERVICE_EXTENSION, insightIntentInfo.serviceExtensionProfileInfo,
        false, g_parseResult);
    GetObjectValueIfFindKey(jsonObject, INSIGHT_INTENT_FORM, insightIntentInfo.formProfileInfo, false, g_parseResult);
}

void from_json(const cJSON *jsonObject, InsightIntentProfileInfoVec &infos)
{
    GetObjectValuesIfFindKey(jsonObject, INSIGHT_INTENTS, infos.insightIntents, false, g_parseResult);
}

bool TransformToInsightIntentInfo(const InsightIntentProfileInfo &insightIntent, InsightIntentInfo &info)
{
    if (insightIntent.intentName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "empty intentName");
        return false;
    }

    info.intentName = insightIntent.intentName;
    info.intentDomain = insightIntent.intentDomain;
    info.intentVersion = insightIntent.intentVersion;
    info.srcEntry = insightIntent.srcEntry;

    info.uiAbilityIntentInfo.abilityName = insightIntent.uiAbilityProfileInfo.abilityName;
    for (const auto &executeMode: insightIntent.uiAbilityProfileInfo.supportExecuteMode) {
        auto mode = std::find_if(std::begin(executeModeMap), std::end(executeModeMap),
            [&executeMode](const auto &item) {
                return item.first == executeMode;
            });
        if (mode == executeModeMap.end()) {
            continue;
        }
        info.uiAbilityIntentInfo.supportExecuteMode.emplace_back(mode->second);
    }

    info.uiExtensionIntentInfo.abilityName = insightIntent.uiExtensionProfileInfo.abilityName;
    info.serviceExtensionIntentInfo.abilityName = insightIntent.serviceExtensionProfileInfo.abilityName;
    info.formIntentInfo.abilityName = insightIntent.formProfileInfo.abilityName;
    info.formIntentInfo.formName = insightIntent.formProfileInfo.formName;
    return true;
}

bool TransformToInfos(const InsightIntentProfileInfoVec &profileInfos, std::vector<InsightIntentInfo> &intentInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    for (const auto &insightIntent : profileInfos.insightIntents) {
        InsightIntentInfo info;
        if (!TransformToInsightIntentInfo(insightIntent, info)) {
            return false;
        }
        intentInfos.push_back(info);
    }
    return true;
}
} // namespace

bool InsightIntentProfile::TransformTo(const std::string &profileStr, std::vector<InsightIntentInfo> &intentInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    cJSON *jsonObject = cJSON_Parse(profileStr.c_str());
    if (jsonObject == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "discarded jsonObject");
        return false;
    }

    InsightIntentProfileInfoVec profileInfos;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_parseResult = ERR_OK;
        from_json(jsonObject, profileInfos);
        if (g_parseResult != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "g_parseResult :%{public}d", g_parseResult);
            int32_t ret = g_parseResult;
            // need recover parse result to ERR_OK
            g_parseResult = ERR_OK;
            cJSON_Delete(jsonObject);
            return ret;
        }
    }
    cJSON_Delete(jsonObject);
    return TransformToInfos(profileInfos, intentInfos);
}
} // namespace AbilityRuntime
} // namespace OHOS
