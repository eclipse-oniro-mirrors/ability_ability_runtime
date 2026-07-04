/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "intent_json_safe_get.h"
#include "json_util.h"

namespace OHOS {
namespace AbilityRuntime {
using JsonType = AppExecFwk::JsonType;
using ArrayType = AppExecFwk::ArrayType;

namespace {
int32_t g_parseResult = ERR_OK;
std::mutex g_mutex;

const std::string INSIGHT_INTENTS = "insightIntents";
const std::string INSIGHT_INTENT_NAME = "intentName";
const std::string INSIGHT_INTENT_DOMAIN = "domain";
const std::string INSIGHT_INTENT_VERSION = "intentVersion";
const std::string INSIGHT_INTENT_SRC_ENTRY = "srcEntry";
const std::string INSIGHT_INTENT_ARKTS_MODE = "arkTSMode";
const std::string INSIGHT_INTENT_UI_ABILITY = "uiAbility";
const std::string INSIGHT_INTENT_UI_EXTENSION = "uiExtension";
const std::string INSIGHT_INTENT_SERVICE_EXTENSION = "serviceExtension";
const std::string INSIGHT_INTENT_FORM = "form";
const std::string INSIGHT_INTENT_ABILITY = "ability";
const std::string INSIGHT_INTENT_EXECUTE_MODE = "executeMode";
const std::string INSIGHT_INTENT_FORM_NAME = "formName";
const std::string INSIGHT_INTENT_DISPLAY_NAME = "displayName";
const std::string INSIGHT_INTENT_DISPLAY_DESCRIPTION = "displayDescription";
const std::string INSIGHT_INTENT_ICON = "icon";
const std::string INSIGHT_INTENT_KEYWORDS = "keywords";
const std::string INSIGHT_INTENT_BUNDLE_NAME = "bundleName";
const std::string INSIGHT_INTENT_MODULE_NAME = "moduleName";
const std::string INSIGHT_INTENT_INPUT_PARAMS = "inputParams";
const std::string INSIGHT_INTENT_OUTPUT_PARAMS = "outputParams";
const std::string INSIGHT_INTENT_ENTITES = "entities";

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
    std::string arkTSMode;
    std::vector<std::string> inputParams;
    std::vector<std::string> outputParams;
    std::string cfgEntities;
    std::string displayName;
    std::string icon;
    std::string displayDescription;
    std::string bundleName;
    std::string moduleName;
    std::vector<std::string> keywords;
    UIAbilityProfileInfo uiAbilityProfileInfo;
    UIExtensionProfileInfo uiExtensionProfileInfo;
    ServiceExtensionProfileInfo serviceExtensionProfileInfo;
    FormProfileInfo formProfileInfo;
};

struct InsightIntentProfileInfoVec {
    std::vector<InsightIntentProfileInfo> insightIntents {};
};

void from_json(const nlohmann::json &jsonObject, UIAbilityProfileInfo &info)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY,
        info.abilityName,
        true,
        g_parseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_EXECUTE_MODE,
        info.supportExecuteMode,
        JsonType::ARRAY,
        true,
        g_parseResult,
        ArrayType::STRING);
}

void from_json(const nlohmann::json &jsonObject, UIExtensionProfileInfo &info)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY,
        info.abilityName,
        true,
        g_parseResult);
}

void from_json(const nlohmann::json &jsonObject, ServiceExtensionProfileInfo &info)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY,
        info.abilityName,
        true,
        g_parseResult);
}

void from_json(const nlohmann::json &jsonObject, FormProfileInfo &info)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY,
        info.abilityName,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FORM_NAME,
        info.formName,
        true,
        g_parseResult);
}

bool ParseParamsElement(const nlohmann::json &param, std::string &errorMsg)
{
    if (!param.is_object()) {
        errorMsg = "type error: inputParams or outputParams element not object";
        return false;
    }
    return true;
}

void ProcessIntputParams(const nlohmann::json &jsonObject,
    InsightIntentProfileInfo &insightIntentInfo, int32_t &g_parseResult)
{
    const auto &jsonObjectEnd = jsonObject.end();
    
    if (jsonObject.find(INSIGHT_INTENT_INPUT_PARAMS) == jsonObjectEnd) {
        return;
    }
    
    const auto &inputParamsJson = jsonObject.at(INSIGHT_INTENT_INPUT_PARAMS);
    if (!inputParamsJson.is_array()) {
        TAG_LOGE(AAFwkTag::INTENT, "type error: inputParams not array");
        g_parseResult = ERR_INVALID_VALUE;
        return;
    }
    
    insightIntentInfo.inputParams.clear();
    std::string errorMsg;
    for (const auto &param : inputParamsJson) {
        if (!ParseParamsElement(param, errorMsg)) {
            TAG_LOGE(AAFwkTag::INTENT, "%{public}s", errorMsg.c_str());
            g_parseResult = ERR_INVALID_VALUE;
            break;
        }
        insightIntentInfo.inputParams.emplace_back(SafeDump(param));
    }
}

void ProcessOutputParams(const nlohmann::json &jsonObject,
    InsightIntentProfileInfo &insightIntentInfo, int32_t &g_parseResult)
{
    const auto &jsonObjectEnd = jsonObject.end();
    
    if (jsonObject.find(INSIGHT_INTENT_OUTPUT_PARAMS) == jsonObjectEnd) {
        return;
    }
    
    const auto &outputParamsJson = jsonObject.at(INSIGHT_INTENT_OUTPUT_PARAMS);
    if (!outputParamsJson.is_array()) {
        TAG_LOGE(AAFwkTag::INTENT, "type error: outputParams not array");
        g_parseResult = ERR_INVALID_VALUE;
        return;
    }
    
    insightIntentInfo.outputParams.clear();
    std::string errorMsg;
    for (const auto &param : outputParamsJson) {
        if (!ParseParamsElement(param, errorMsg)) {
            TAG_LOGE(AAFwkTag::INTENT, "%{public}s", errorMsg.c_str());
            g_parseResult = ERR_INVALID_VALUE;
            break;
        }
        insightIntentInfo.outputParams.emplace_back(SafeDump(param));
    }
}

void from_json(const nlohmann::json &jsonObject, InsightIntentProfileInfo &insightIntentInfo)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAME,
        insightIntentInfo.intentName,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DOMAIN,
        insightIntentInfo.intentDomain,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_VERSION,
        insightIntentInfo.intentVersion,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_SRC_ENTRY,
        insightIntentInfo.srcEntry,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ARKTS_MODE,
        insightIntentInfo.arkTSMode,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DISPLAY_NAME,
        insightIntentInfo.displayName,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DISPLAY_DESCRIPTION,
        insightIntentInfo.displayDescription,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ICON,
        insightIntentInfo.icon,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_BUNDLE_NAME,
        insightIntentInfo.bundleName,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_MODULE_NAME,
        insightIntentInfo.moduleName,
        false,
        g_parseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_KEYWORDS,
        insightIntentInfo.keywords,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::STRING);
    AppExecFwk::GetValueIfFindKey<UIAbilityProfileInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_UI_ABILITY,
        insightIntentInfo.uiAbilityProfileInfo,
        JsonType::OBJECT,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    AppExecFwk::GetValueIfFindKey<UIExtensionProfileInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_UI_EXTENSION,
        insightIntentInfo.uiExtensionProfileInfo,
        JsonType::OBJECT,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    AppExecFwk::GetValueIfFindKey<ServiceExtensionProfileInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_SERVICE_EXTENSION,
        insightIntentInfo.serviceExtensionProfileInfo,
        JsonType::OBJECT,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);
    AppExecFwk::GetValueIfFindKey<FormProfileInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FORM,
        insightIntentInfo.formProfileInfo,
        JsonType::OBJECT,
        false,
        g_parseResult,
        ArrayType::NOT_ARRAY);

    ProcessIntputParams(jsonObject, insightIntentInfo, g_parseResult);
    ProcessOutputParams(jsonObject, insightIntentInfo, g_parseResult);
    if (jsonObject.find(INSIGHT_INTENT_ENTITES) != jsonObjectEnd) {
        if (jsonObject.at(INSIGHT_INTENT_ENTITES).is_object()) {
            insightIntentInfo.cfgEntities = SafeDump(jsonObject[INSIGHT_INTENT_ENTITES]);
        } else {
            TAG_LOGE(AAFwkTag::INTENT, "type error: cfgEntities not object");
            g_parseResult = ERR_INVALID_VALUE;
        }
    }
}

void from_json(const nlohmann::json &jsonObject, InsightIntentProfileInfoVec &infos)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::GetValueIfFindKey<std::vector<InsightIntentProfileInfo>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENTS,
        infos.insightIntents,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);
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
    info.arkTSMode = insightIntent.arkTSMode;
    info.inputParams.assign(insightIntent.inputParams.begin(), insightIntent.inputParams.end());
    info.outputParams.assign(insightIntent.outputParams.begin(), insightIntent.outputParams.end());
    info.displayName = insightIntent.displayName;
    info.bundleName = insightIntent.bundleName;
    info.moduleName = insightIntent.moduleName;
    info.displayDescription = insightIntent.displayDescription;
    info.icon = insightIntent.icon;
    info.keywords.assign(insightIntent.keywords.begin(), insightIntent.keywords.end());
    info.cfgEntities = insightIntent.cfgEntities;

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
    auto jsonObject = nlohmann::json::parse(profileStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "discarded jsonObject");
        return false;
    }
    TAG_LOGD(AAFwkTag::INTENT, "jsonObject : %{public}s", SafeDump(jsonObject, DUMP_LOG_MAX_LEN).c_str());

    InsightIntentProfileInfoVec profileInfos;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_parseResult = ERR_OK;
        if (!SafeJsonGet(jsonObject, profileInfos, "InsightIntentProfileInfoVec")) {
            g_parseResult = ERR_OK;
            return false;
        }
        if (g_parseResult != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "g_parseResult :%{public}d", g_parseResult);
            g_parseResult = ERR_OK;
            return false;
        }
    }

    return TransformToInfos(profileInfos, intentInfos);
}

void to_json(nlohmann::json& jsonObject, const UIAbilityIntentInfo& info)
{
    std::vector<std::string> modes;
    for (auto m : info.supportExecuteMode) {
        if (m == ExecuteMode::UI_ABILITY_FOREGROUND) modes.emplace_back("foreground");
        else if (m == ExecuteMode::UI_ABILITY_BACKGROUND) modes.emplace_back("background");
    }
    jsonObject = {
        {INSIGHT_INTENT_ABILITY, info.abilityName},
        {INSIGHT_INTENT_EXECUTE_MODE, modes}
    };
}

void to_json(nlohmann::json& jsonObject, const UIExtensionIntentInfo& info)
{
    jsonObject = {{INSIGHT_INTENT_ABILITY, info.abilityName}};
}

void to_json(nlohmann::json& jsonObject, const ServiceExtensionIntentInfo& info)
{
    jsonObject = {{INSIGHT_INTENT_ABILITY, info.abilityName}};
}

void to_json(nlohmann::json& jsonObject, const FormIntentInfo& info)
{
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_ABILITY, info.abilityName},
        {INSIGHT_INTENT_FORM_NAME, info.formName}
    };
}

void to_json(nlohmann::json& jsonObject, const InsightIntentInfo& info)
{
    TAG_LOGI(AAFwkTag::INTENT, "InsightIntentInfo to json");

    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_BUNDLE_NAME, info.bundleName},
        {INSIGHT_INTENT_MODULE_NAME, info.moduleName},
        {INSIGHT_INTENT_NAME, info.intentName},
        {INSIGHT_INTENT_DOMAIN, info.intentDomain},
        {INSIGHT_INTENT_VERSION, info.intentVersion},
        {INSIGHT_INTENT_ARKTS_MODE, info.arkTSMode},
        {INSIGHT_INTENT_SRC_ENTRY, info.srcEntry},
        {INSIGHT_INTENT_DISPLAY_NAME, info.displayName},
        {INSIGHT_INTENT_DISPLAY_DESCRIPTION, info.displayDescription},
        {INSIGHT_INTENT_ICON, info.icon},
        {INSIGHT_INTENT_KEYWORDS, info.keywords},
        {INSIGHT_INTENT_UI_ABILITY, info.uiAbilityIntentInfo},
        {INSIGHT_INTENT_UI_EXTENSION, info.uiExtensionIntentInfo},
        {INSIGHT_INTENT_SERVICE_EXTENSION, info.serviceExtensionIntentInfo},
        {INSIGHT_INTENT_FORM, info.formIntentInfo}
    };
    nlohmann::json inputArray = nlohmann::json::array();
    for (const auto &paramStr : info.inputParams) {
        if (paramStr.empty()) {
            continue;
        }
        auto paramJson = nlohmann::json::parse(paramStr, nullptr, false);
        if (!paramJson.is_discarded()) {
            inputArray.emplace_back(paramJson);
        }
    }
    jsonObject[INSIGHT_INTENT_INPUT_PARAMS] = inputArray;
    nlohmann::json outputArray = nlohmann::json::array();
    for (const auto &paramStr : info.outputParams) {
        if (paramStr.empty()) {
            continue;
        }
        auto paramJson = nlohmann::json::parse(paramStr, nullptr, false);
        if (!paramJson.is_discarded()) {
            outputArray.emplace_back(paramJson);
        }
    }
    jsonObject[INSIGHT_INTENT_OUTPUT_PARAMS] = outputArray;
    if (!info.cfgEntities.empty()) {
        auto cfgEntities = nlohmann::json::parse(info.cfgEntities, nullptr, false);
        if (cfgEntities.is_discarded()) {
            TAG_LOGE(AAFwkTag::INTENT, "discarded entity parameters");
            return;
        }

        jsonObject[INSIGHT_INTENT_ENTITES] = cfgEntities;
    }
}

bool InsightIntentProfile::ToJson(const InsightIntentInfo &info, nlohmann::json &jsonObject)
{
    TAG_LOGD(AAFwkTag::INTENT, "to json");
    nlohmann::json subJsonObject = info;
    if (subJsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "bad insight intent info");
        return false;
    }

    jsonObject[INSIGHT_INTENTS] = nlohmann::json::array({ subJsonObject });
    TAG_LOGD(AAFwkTag::INTENT, "to json string: %{public}s", SafeDump(jsonObject, DUMP_LOG_MAX_LEN).c_str());
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
