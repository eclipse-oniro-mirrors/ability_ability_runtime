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

#include "extract_insight_intent_profile.h"

#include <algorithm>

#include "hilog_tag_wrapper.h"
#include "json_util.h"

namespace OHOS {
namespace AbilityRuntime {
using JsonType = AppExecFwk::JsonType;
using ArrayType = AppExecFwk::ArrayType;

namespace {
int32_t g_extraParseResult = ERR_OK;
std::mutex g_extraMutex;

const std::string INSIGHT_INTENTS = "extractInsightIntents";
const std::string INSIGHT_INTENT_NAME = "intentName";
const std::string INSIGHT_INTENT_DOMAIN = "domain";
const std::string INSIGHT_INTENT_VERSION = "intentVersion";
const std::string INSIGHT_INTENT_EXECUTE_MODE = "executeMode";
const std::string INSIGHT_INTENT_DECORETOR_FILE = "decoratorFile";
const std::string INSIGHT_INTENT_DECORETOR_CLASS = "decoratorClass";
const std::string INSIGHT_INTENT_DECORETOR_TYPE = "decoratorType";
const std::string INSIGHT_INTENT_BUNDLE_NAME = "bundleName";
const std::string INSIGHT_INTENT_MODULE_NAME = "moduleName";
const std::string INSIGHT_INTENT_DISPLAY_NAME = "displayName";
const std::string INSIGHT_INTENT_DISPLAY_DESCRIPTION = "displayDescription";
const std::string INSIGHT_INTENT_SCHEMA = "schema";
const std::string INSIGHT_INTENT_ICON = "icon";
const std::string INSIGHT_INTENT_LLM_DESCRIPTION = "llmDescription";
const std::string INSIGHT_INTENT_KEYWORDS = "keywords";
const std::string INSIGHT_INTENT_PARAMETERS = "parameters";
const std::string INSIGHT_INTENT_URI = "uri";
const std::string INSIGHT_INTENT_PARAM_MAPPING = "paramMappings";
const std::string INSIGHT_INTENT_UI_ABILITY = "uiAbility";
const std::string INSIGHT_INTENT_PAGE_ROUTE_NAME = "pagePath";
const std::string INSIGHT_INTENT_NAVIGATION_ID = "navigationId";
const std::string INSIGHT_INTENT_NAV_DESTINATION_NAME = "navDestinationName";
const std::string INSIGHT_INTENT_ABILITY_NAME = "abilityName";
const std::string INSIGHT_INTENT_FUNCTION_NAME = "functionName";
const std::string INSIGHT_INTENT_FUNCTION_RETURN_TYPE = "functionReturnType";
const std::string INSIGHT_INTENT_FUNCTION_PARAMS = "functionParamList";
const std::string INSIGHT_INTENT_PARAM_NAME = "paramName";
const std::string INSIGHT_INTENT_PARAM_MAPPING_NAME = "paramMappingName";
const std::string INSIGHT_INTENT_PARAM_CATEGORY = "paramCategory";
const std::string INSIGHT_INTENT_RESULT = "result";
const std::string INSIGHT_INTENT_EXAMPLE = "example";
const std::string INSIGHT_INTENT_FORM_NAME = "formName";
const std::string INSIGHT_INTENT_ENTITES = "entities";
const std::string INSIGHT_INTENT_ENTITY_DECORETOR_FILE = "decoratorFile";
const std::string INSIGHT_INTENT_ENTITY_CLASS_NAME = "className";
const std::string INSIGHT_INTENT_ENTITY_DECORETOR_TYPE = "decoratorType";
const std::string INSIGHT_INTENT_ENTITY_ID = "entityId";
const std::string INSIGHT_INTENT_ENTITY_CATEGORY = "entityCategory";
const std::string INSIGHT_INTENT_ENTITY_PARENT_CLASS_NAME = "parentClassName";
const std::string INSIGHT_INTENT_ENTITY_PARAMETERS = "parameters";
const std::string INSIGHT_INTENT_ARKTS_MODE = "arkTSMode";
const std::string INSIGHT_INTENT_ENTITY_SUPPORTED_QUERY_PROPERTIES = "supportedQueryProperties";

enum DecoratorType {
    DECORATOR_LINK = 0,
    DECORATOR_PAGE,
    DECORATOR_ENTRY,
    DECORATOR_FUNCTION,
    DECORATOR_FORM,
    DECORATOR_UNKNOWN
};

DecoratorType StringToEnum(const std::string& input)
{
    static const std::unordered_map<std::string, DecoratorType> mapping = {
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_LINK, DecoratorType::DECORATOR_LINK},
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_PAGE, DecoratorType::DECORATOR_PAGE},
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY, DecoratorType::DECORATOR_ENTRY},
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION, DecoratorType::DECORATOR_FUNCTION},
        {AbilityRuntime::INSIGHT_INTENTS_DECORATOR_TYPE_FORM, DecoratorType::DECORATOR_FORM}
    };
    auto it = mapping.find(input);
    return (it != mapping.end()) ? it->second : DecoratorType::DECORATOR_UNKNOWN;
}

const std::map<std::string, ExecuteMode> executeModeMap = {
    {"foreground", ExecuteMode::UI_ABILITY_FOREGROUND},
    {"background", ExecuteMode::UI_ABILITY_BACKGROUND},
    {"uiextension", ExecuteMode::UI_EXTENSION_ABILITY},
    {"serviceextension", ExecuteMode::SERVICE_EXTENSION_ABILITY}
};
} // namespace

void from_json(const nlohmann::json &jsonObject, LinkIntentParamProfileMapping &paramMapping)
{
    TAG_LOGD(AAFwkTag::INTENT, "LinkIntentParamProfileMapping from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAM_NAME,
        paramMapping.paramName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAM_MAPPING_NAME,
        paramMapping.paramMappingName,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAM_CATEGORY,
        paramMapping.paramCategory,
        false,
        g_extraParseResult);
}

void from_json(const nlohmann::json &jsonObject, InsightIntentEntityInfo &entityInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightIntentEntityInfo from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_DECORETOR_FILE,
        entityInfo.decoratorFile,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_CLASS_NAME,
        entityInfo.className,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_DECORETOR_TYPE,
        entityInfo.decoratorType,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_ID,
        entityInfo.entityId,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_CATEGORY,
        entityInfo.entityCategory,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_PARENT_CLASS_NAME,
        entityInfo.parentClassName,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_SUPPORTED_QUERY_PROPERTIES,
        entityInfo.supportedQueryProperties,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::STRING);

    if (jsonObject.find(INSIGHT_INTENT_ENTITY_PARAMETERS) != jsonObjectEnd) {
        if (jsonObject.at(INSIGHT_INTENT_ENTITY_PARAMETERS).is_object()) {
            entityInfo.parameters = jsonObject[INSIGHT_INTENT_ENTITY_PARAMETERS].dump();
        } else {
            TAG_LOGE(AAFwkTag::INTENT, "type error: entity parameters not object");
            g_extraParseResult = ERR_INVALID_VALUE;
        }
    }
}

void from_json(const nlohmann::json &jsonObject, ExtractInsightIntentProfileInfo &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "ExtractInsightIntentProfileInfo from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DECORETOR_FILE,
        insightIntentInfo.decoratorFile,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DECORETOR_CLASS,
        insightIntentInfo.decoratorClass,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DECORETOR_TYPE,
        insightIntentInfo.decoratorType,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_BUNDLE_NAME,
        insightIntentInfo.bundleName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_MODULE_NAME,
        insightIntentInfo.moduleName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAME,
        insightIntentInfo.intentName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DOMAIN,
        insightIntentInfo.domain,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_VERSION,
        insightIntentInfo.intentVersion,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DISPLAY_NAME,
        insightIntentInfo.displayName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DISPLAY_DESCRIPTION,
        insightIntentInfo.displayDescription,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_SCHEMA,
        insightIntentInfo.schema,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ICON,
        insightIntentInfo.icon,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_LLM_DESCRIPTION,
        insightIntentInfo.llmDescription,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_KEYWORDS,
        insightIntentInfo.keywords,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::STRING);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_EXAMPLE,
        insightIntentInfo.example,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_URI,
        insightIntentInfo.uri,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<LinkIntentParamProfileMapping>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAM_MAPPING,
        insightIntentInfo.paramMapping,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::OBJECT);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_UI_ABILITY,
        insightIntentInfo.uiAbility,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PAGE_ROUTE_NAME,
        insightIntentInfo.pagePath,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAVIGATION_ID,
        insightIntentInfo.navigationId,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAV_DESTINATION_NAME,
        insightIntentInfo.navDestinationName,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY_NAME,
        insightIntentInfo.abilityName,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_EXECUTE_MODE,
        insightIntentInfo.executeMode,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::STRING);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FUNCTION_NAME,
        insightIntentInfo.functionName,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FUNCTION_RETURN_TYPE,
        insightIntentInfo.functionReturnType,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FUNCTION_PARAMS,
        insightIntentInfo.functionParams,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::STRING);
    AppExecFwk::GetValueIfFindKey<std::vector<InsightIntentEntityInfo>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITES,
        insightIntentInfo.entities,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::OBJECT);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FORM_NAME,
        insightIntentInfo.formName,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ARKTS_MODE,
        insightIntentInfo.arkTSMode,
        false,
        g_extraParseResult);

    if (jsonObject.find(INSIGHT_INTENT_PARAMETERS) != jsonObjectEnd) {
        if (jsonObject.at(INSIGHT_INTENT_PARAMETERS).is_object()) {
            insightIntentInfo.parameters =  jsonObject[INSIGHT_INTENT_PARAMETERS].dump();
        } else {
            TAG_LOGE(AAFwkTag::INTENT, "type error: parameters not object");
            g_extraParseResult = ERR_INVALID_VALUE;
        }
    }

    if (jsonObject.find(INSIGHT_INTENT_RESULT) != jsonObjectEnd) {
        if (jsonObject.at(INSIGHT_INTENT_RESULT).is_object()) {
            insightIntentInfo.result =  jsonObject[INSIGHT_INTENT_RESULT].dump();
        } else {
            TAG_LOGE(AAFwkTag::INTENT, "type error: result not object");
            g_extraParseResult = ERR_INVALID_VALUE;
        }
    }
}

void from_json(const nlohmann::json &jsonObject, ExtractInsightIntentProfileInfoVec &infos)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::GetValueIfFindKey<std::vector<ExtractInsightIntentProfileInfo>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENTS,
        infos.insightIntents,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::OBJECT);
}

void to_json(nlohmann::json& jsonObject, const LinkIntentParamProfileMapping &info)
{
    TAG_LOGI(AAFwkTag::INTENT, "LinkIntentParamProfileMapping to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_PARAM_NAME, info.paramName},
        {INSIGHT_INTENT_PARAM_MAPPING_NAME, info.paramMappingName},
        {INSIGHT_INTENT_PARAM_CATEGORY, info.paramCategory}
    };
}

void to_json(nlohmann::json& jsonObject, const InsightIntentEntityInfo &info)
{
    TAG_LOGI(AAFwkTag::INTENT, "InsightIntentEntityInfo to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_ENTITY_DECORETOR_FILE, info.decoratorFile},
        {INSIGHT_INTENT_ENTITY_CLASS_NAME, info.className},
        {INSIGHT_INTENT_ENTITY_DECORETOR_TYPE, info.decoratorType},
        {INSIGHT_INTENT_ENTITY_ID, info.entityId},
        {INSIGHT_INTENT_ENTITY_CATEGORY, info.entityCategory},
        {INSIGHT_INTENT_ENTITY_PARENT_CLASS_NAME, info.parentClassName},
        {INSIGHT_INTENT_ENTITY_SUPPORTED_QUERY_PROPERTIES, info.supportedQueryProperties},
    };

    if (!info.parameters.empty()) {
        auto parameters = nlohmann::json::parse(info.parameters, nullptr, false);
        if (parameters.is_discarded()) {
            TAG_LOGE(AAFwkTag::INTENT, "discarded entity parameters");
            return;
        }

        jsonObject[INSIGHT_INTENT_ENTITY_PARAMETERS] = parameters;
    }
}

void to_json(nlohmann::json& jsonObject, const ExtractInsightIntentProfileInfo& info)
{
    TAG_LOGI(AAFwkTag::INTENT, "ExtractInsightIntentProfileInfo to json");

    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_DECORETOR_FILE, info.decoratorFile},
        {INSIGHT_INTENT_DECORETOR_CLASS, info.decoratorClass},
        {INSIGHT_INTENT_DECORETOR_TYPE, info.decoratorType},
        {INSIGHT_INTENT_BUNDLE_NAME, info.bundleName},
        {INSIGHT_INTENT_MODULE_NAME, info.moduleName},
        {INSIGHT_INTENT_NAME, info.intentName},
        {INSIGHT_INTENT_DOMAIN, info.domain},
        {INSIGHT_INTENT_VERSION, info.intentVersion},
        {INSIGHT_INTENT_DISPLAY_NAME, info.displayName},
        {INSIGHT_INTENT_DISPLAY_DESCRIPTION, info.displayDescription},
        {INSIGHT_INTENT_SCHEMA, info.schema},
        {INSIGHT_INTENT_ICON, info.icon},
        {INSIGHT_INTENT_LLM_DESCRIPTION, info.llmDescription},
        {INSIGHT_INTENT_KEYWORDS, info.keywords},
        {INSIGHT_INTENT_EXAMPLE, info.example},
        {INSIGHT_INTENT_URI, info.uri},
        {INSIGHT_INTENT_PARAM_MAPPING, info.paramMapping},
        {INSIGHT_INTENT_UI_ABILITY, info.uiAbility},
        {INSIGHT_INTENT_PAGE_ROUTE_NAME, info.pagePath},
        {INSIGHT_INTENT_NAVIGATION_ID, info.navigationId},
        {INSIGHT_INTENT_NAV_DESTINATION_NAME, info.navDestinationName},
        {INSIGHT_INTENT_ABILITY_NAME, info.abilityName},
        {INSIGHT_INTENT_EXECUTE_MODE, info.executeMode},
        {INSIGHT_INTENT_FUNCTION_NAME, info.functionName},
        {INSIGHT_INTENT_FUNCTION_RETURN_TYPE, info.functionReturnType},
        {INSIGHT_INTENT_FUNCTION_PARAMS, info.functionParams},
        {INSIGHT_INTENT_FORM_NAME, info.formName},
        {INSIGHT_INTENT_ENTITES, info.entities},
        {INSIGHT_INTENT_ARKTS_MODE, info.arkTSMode}
    };

    if (!info.parameters.empty()) {
        auto parameters = nlohmann::json::parse(info.parameters, nullptr, false);
        if (parameters.is_discarded()) {
            TAG_LOGE(AAFwkTag::INTENT, "discarded parameters");
            return;
        }

        jsonObject[INSIGHT_INTENT_PARAMETERS] = parameters;
    }

    if (!info.result.empty()) {
        auto result = nlohmann::json::parse(info.result, nullptr, false);
        if (result.is_discarded()) {
            TAG_LOGE(AAFwkTag::INTENT, "discarded result");
            return;
        }

        jsonObject[INSIGHT_INTENT_RESULT] = result;
    }
}

bool CheckProfileSubIntentInfo(const ExtractInsightIntentProfileInfo &insightIntent)
{
    switch (StringToEnum(insightIntent.decoratorType)) {
        case DecoratorType::DECORATOR_LINK:
            if (insightIntent.uri.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty uri, intentName: %{public}s", insightIntent.intentName.c_str());
                return false;
            }
            for (const auto &paramMapping: insightIntent.paramMapping) {
                if (paramMapping.paramName.empty()) {
                    TAG_LOGE(AAFwkTag::INTENT, "empty paramName, intentName: %{public}s",
                             insightIntent.intentName.c_str());
                    return false;
                }
            }
            break;
        case DecoratorType::DECORATOR_PAGE:
            if (insightIntent.pagePath.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty pagePath, intentName: %{public}s",
                         insightIntent.intentName.c_str());
                return false;
            }
            break;
        case DecoratorType::DECORATOR_ENTRY:
            if (insightIntent.abilityName.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty abilityName, intentName: %{public}s",
                         insightIntent.intentName.c_str());
                return false;
            }
            break;
        case DecoratorType::DECORATOR_FUNCTION:
            if (insightIntent.functionName.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty functionName, intentName: %{public}s",
                         insightIntent.intentName.c_str());
                return false;
            }
            break;
        case DecoratorType::DECORATOR_FORM:
            if (insightIntent.formName.empty() || insightIntent.abilityName.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty formName or abilityName, intentName: %{public}s, "
                    "abilityName: %{public}s", insightIntent.intentName.c_str(), insightIntent.abilityName.c_str());
                return false;
            }
            break;
        default:
            TAG_LOGE(AAFwkTag::INTENT, "invalid decoratorType: %{public}s", insightIntent.decoratorType.c_str());
            return false;
    }

    return true;
}

bool CheckProfileInfo(const ExtractInsightIntentProfileInfo &insightIntent)
{
    if (insightIntent.decoratorFile.empty() || insightIntent.decoratorClass.empty() ||
        insightIntent.decoratorType.empty() || insightIntent.bundleName.empty() ||
        insightIntent.moduleName.empty() || insightIntent.intentName.empty() || insightIntent.domain.empty() ||
        insightIntent.intentVersion.empty() || insightIntent.displayName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "exist empty param, decoratorFile: %{public}s, decoratorClass: %{public}s, "
            "decoratorType: %{public}s, bundleName: %{public}s, moduleName: %{public}s, intentName: %{public}s, "
            "domain: %{public}s, intentVersion: %{public}s, displayName: %{public}s",
            insightIntent.decoratorFile.c_str(), insightIntent.decoratorClass.c_str(),
            insightIntent.decoratorType.c_str(), insightIntent.bundleName.c_str(), insightIntent.moduleName.c_str(),
            insightIntent.intentName.c_str(), insightIntent.domain.c_str(), insightIntent.intentVersion.c_str(),
            insightIntent.displayName.c_str());
        return false;
    }

    for (const auto &entity: insightIntent.entities) {
        if (entity.className.empty() || (entity.entityId.empty() && !entity.isQueryable())) {
            TAG_LOGE(AAFwkTag::INTENT, "entity exist empty param, intentName: %{public}s, "
                "className: %{public}s, entityId: %{public}s",
                insightIntent.intentName.c_str(), entity.className.c_str(), entity.entityId.c_str());
            return false;
        }
    }

    return CheckProfileSubIntentInfo(insightIntent);
}

bool TransformToLinkInfo(const ExtractInsightIntentProfileInfo &insightIntent, InsightIntentLinkInfo &info)
{
    info.uri = insightIntent.uri;
    TAG_LOGD(AAFwkTag::INTENT, "uri: %{public}s", info.uri.c_str());
    for (std::vector<LinkIntentParamProfileMapping>::const_iterator iter = insightIntent.paramMapping.begin();
        iter != insightIntent.paramMapping.end(); iter++) {
        LinkIntentParamMapping paramMapping;
        paramMapping.paramName = (*iter).paramName;
        TAG_LOGD(AAFwkTag::INTENT, "paramName: %{public}s", paramMapping.paramName.c_str());
        paramMapping.paramMappingName = (*iter).paramMappingName;
        TAG_LOGD(AAFwkTag::INTENT, "paramMappingName: %{public}s", paramMapping.paramMappingName.c_str());
        paramMapping.paramCategory = (*iter).paramCategory;
        TAG_LOGD(AAFwkTag::INTENT, "paramCategory: %{public}s", paramMapping.paramCategory.c_str());

        info.paramMapping.push_back(paramMapping);
    }
    info.parameters = insightIntent.parameters;
    TAG_LOGD(AAFwkTag::INTENT, "link parameters: %{public}s", info.parameters.c_str());
    return true;
}

bool TransformToPageInfo(const ExtractInsightIntentProfileInfo &insightIntent, InsightIntentPageInfo &info)
{
    info.uiAbility = insightIntent.uiAbility;
    TAG_LOGD(AAFwkTag::INTENT, "uiAbility: %{public}s", info.uiAbility.c_str());
    info.pagePath = insightIntent.pagePath;
    TAG_LOGD(AAFwkTag::INTENT, "pagePath: %{public}s", info.pagePath.c_str());
    info.navigationId = insightIntent.navigationId;
    TAG_LOGD(AAFwkTag::INTENT, "navigationId: %{public}s", info.navigationId.c_str());
    info.navDestinationName = insightIntent.navDestinationName;
    TAG_LOGD(AAFwkTag::INTENT, "navDestinationName: %{public}s", info.navDestinationName.c_str());
    info.parameters = insightIntent.parameters;
    TAG_LOGD(AAFwkTag::INTENT, "page parameters: %{public}s", info.parameters.c_str());
    return true;
}

bool TransformToEntryInfo(const ExtractInsightIntentProfileInfo &insightIntent, InsightIntentEntryInfo &info)
{
    info.abilityName = insightIntent.abilityName;
    for (const auto &executeMode: insightIntent.executeMode) {
        auto mode = std::find_if(std::begin(executeModeMap), std::end(executeModeMap),
            [&executeMode](const auto &item) {
                return item.first == executeMode;
            });
        if (mode == executeModeMap.end()) {
            TAG_LOGW(AAFwkTag::INTENT, "not support execute mode: %{public}s", executeMode.c_str());
            continue;
        }
        info.executeMode.emplace_back(mode->second);
        TAG_LOGI(AAFwkTag::INTENT, "mode: %{public}s", mode->first.c_str());
    }
    info.parameters = insightIntent.parameters;
    TAG_LOGD(AAFwkTag::INTENT, "entry parameters: %{public}s", info.parameters.c_str());
    return true;
}

bool TransformToFunctionInfo(const ExtractInsightIntentProfileInfo &insightIntent, InsightIntentFunctionInfo &info)
{
    info.functionName = insightIntent.functionName;
    TAG_LOGD(AAFwkTag::INTENT, "functionName: %{public}s", info.functionName.c_str());
    info.functionReturnType = insightIntent.functionReturnType;
    TAG_LOGD(AAFwkTag::INTENT, "functionReturnType: %{public}s", info.functionReturnType.c_str());
    info.functionParams.assign(insightIntent.functionParams.begin(), insightIntent.functionParams.end());
    for (size_t i = 0; i < info.functionParams.size(); i++) {
        TAG_LOGD(AAFwkTag::INTENT, "functionParams[%{public}zu]: %{public}s", i, info.functionParams[i].c_str());
    }
    info.parameters = insightIntent.parameters;
    TAG_LOGD(AAFwkTag::INTENT, "function parameters: %{public}s", info.parameters.c_str());
    return true;
}

bool TransformToFormInfo(const ExtractInsightIntentProfileInfo &insightIntent, InsightIntentFormInfo &info)
{
    info.abilityName = insightIntent.abilityName;
    info.formName = insightIntent.formName;
    info.parameters = insightIntent.parameters;
    TAG_LOGD(AAFwkTag::INTENT, "form parameters: %{public}s", info.parameters.c_str());
    return true;
}

bool ExtractInsightIntentProfile::TransformTo(const std::string &profileStr,
    ExtractInsightIntentProfileInfoVec &intentInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "transform profileStr: %{public}s", profileStr.c_str());
    auto jsonObject = nlohmann::json::parse(profileStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "discarded jsonObject, profileStr: %{public}s", profileStr.c_str());
        return false;
    }

    std::lock_guard<std::mutex> lock(g_extraMutex);
    g_extraParseResult = ERR_OK;
    intentInfos = jsonObject.get<ExtractInsightIntentProfileInfoVec>();
    if (g_extraParseResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "parse result: %{public}d, profileStr: %{public}s",
            g_extraParseResult, profileStr.c_str());
        g_extraParseResult = ERR_OK;
        return false;
    }

    for (const auto &insightIntent : intentInfos.insightIntents) {
        if (!CheckProfileInfo(insightIntent)) {
            return false;
        }
    }

    TAG_LOGD(AAFwkTag::INTENT, "transform success, size: %{public}zu", intentInfos.insightIntents.size());
    return true;
}

bool ExtractInsightIntentProfile::ToJson(const ExtractInsightIntentProfileInfo &info, nlohmann::json &jsonObject)
{
    TAG_LOGD(AAFwkTag::INTENT, "to json");
    nlohmann::json subJsonObject = info;
    if (subJsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "bad insight intent info");
        return false;
    }

    jsonObject[INSIGHT_INTENTS] = nlohmann::json::array({ subJsonObject });
    TAG_LOGD(AAFwkTag::INTENT, "to json string: %{public}s", jsonObject.dump().c_str());
    return true;
}

bool ExtractInsightIntentProfile::ProfileInfoFormat(const ExtractInsightIntentProfileInfo &insightIntent,
    ExtractInsightIntentInfo &info)
{
    bool ret = false;
    if (!CheckProfileInfo(insightIntent)) {
        return false;
    }

    info.decoratorFile = insightIntent.decoratorFile;
    info.decoratorClass = insightIntent.decoratorClass;
    info.displayDescription = insightIntent.displayDescription;
    info.domain = insightIntent.domain;
    info.intentVersion = insightIntent.intentVersion;
    info.schema = insightIntent.schema;
    info.icon = insightIntent.icon;
    info.llmDescription = insightIntent.llmDescription;
    info.example = insightIntent.example;
    info.result = insightIntent.result;
    info.keywords.assign(insightIntent.keywords.begin(), insightIntent.keywords.end());
    info.arkTSMode = insightIntent.arkTSMode;
    info.entities = insightIntent.entities;
    TAG_LOGD(AAFwkTag::INTENT, "entities size: %{public}zu", info.entities.size());
    for (auto iter = info.entities.begin(); iter != info.entities.end(); iter++) {
        TAG_LOGD(AAFwkTag::INTENT, "entity decoratorFile: %{public}s, className: %{public}s, "
            "decoratorType: %{public}s, entityId: %{public}s, entityCategory: %{public}s, "
            "parentClassName: %{public}s, parameters: %{public}s, supportedQueryProperties: %{public}zu",
            (*iter).decoratorFile.c_str(), (*iter).className.c_str(), (*iter).decoratorType.c_str(),
            (*iter).entityId.c_str(), (*iter).entityCategory.c_str(), (*iter).parentClassName.c_str(),
            (*iter).parameters.c_str(), (*iter).supportedQueryProperties.size());
    }

    info.genericInfo.bundleName = insightIntent.bundleName;
    TAG_LOGD(AAFwkTag::INTENT, "bundleName: %{public}s", info.genericInfo.bundleName.c_str());
    info.genericInfo.moduleName = insightIntent.moduleName;
    TAG_LOGD(AAFwkTag::INTENT, "moduleName: %{public}s", info.genericInfo.moduleName.c_str());
    info.genericInfo.intentName = insightIntent.intentName;
    TAG_LOGD(AAFwkTag::INTENT, "intentName: %{public}s", info.genericInfo.intentName.c_str());
    info.genericInfo.displayName = insightIntent.displayName;
    TAG_LOGD(AAFwkTag::INTENT, "displayName: %{public}s", info.genericInfo.displayName.c_str());
    info.genericInfo.decoratorType = insightIntent.decoratorType;
    TAG_LOGD(AAFwkTag::INTENT, "decoratorType: %{public}s", info.genericInfo.decoratorType.c_str());

    switch (StringToEnum(insightIntent.decoratorType)) {
        case DecoratorType::DECORATOR_LINK:
            ret = TransformToLinkInfo(insightIntent, info.genericInfo.get<InsightIntentLinkInfo>());
            break;
        case DecoratorType::DECORATOR_PAGE:
            ret = TransformToPageInfo(insightIntent, info.genericInfo.get<InsightIntentPageInfo>());
            break;
        case DecoratorType::DECORATOR_ENTRY:
            ret = TransformToEntryInfo(insightIntent, info.genericInfo.get<InsightIntentEntryInfo>());
            break;
        case DecoratorType::DECORATOR_FUNCTION:
            ret = TransformToFunctionInfo(insightIntent, info.genericInfo.get<InsightIntentFunctionInfo>());
            break;
        case DecoratorType::DECORATOR_FORM:
            ret = TransformToFormInfo(insightIntent, info.genericInfo.get<InsightIntentFormInfo>());
            break;
        default:
            TAG_LOGE(AAFwkTag::INTENT, "invalid decoratorType: %{public}s", insightIntent.decoratorType.c_str());
            return false;
    }

    return ret;
}
} // namespace AbilityRuntime
} // namespace OHOS
