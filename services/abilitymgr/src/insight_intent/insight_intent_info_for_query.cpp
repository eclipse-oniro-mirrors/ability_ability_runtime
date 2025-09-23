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

#include "insight_intent_info_for_query.h"

#include "string_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "json_util.h"

namespace OHOS {
namespace AbilityRuntime {
using JsonType = AppExecFwk::JsonType;
using ArrayType = AppExecFwk::ArrayType;
namespace {
int32_t g_parseResult = ERR_OK;
std::mutex g_extraMutex;

const std::map<AppExecFwk::ExecuteMode, std::string> EXECUTE_MODE_STRING_MAP = {
    {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND, "UI_ABILITY_FOREGROUND"},
    {AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND, "UI_ABILITY_BACKGROUND"},
    {AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY, "UI_EXTENSION_ABILITY"},
    {AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY, "SERVICE_EXTENSION_ABILITY"}
};
const std::map<std::string, AppExecFwk::ExecuteMode> STRING_EXECUTE_MODE_MAP = {
    {"UI_ABILITY_FOREGROUND", AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND},
    {"UI_ABILITY_BACKGROUND", AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND},
    {"UI_EXTENSION_ABILITY", AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY},
    {"SERVICE_EXTENSION_ABILITY", AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY}
};
}

void from_json(const nlohmann::json &jsonObject, LinkInfoForQuery &linkInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "LinkInfoForQuery from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENTS_URI,
        linkInfo.uri,
        true,
        g_parseResult);
}

void to_json(nlohmann::json& jsonObject, const LinkInfoForQuery &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "LinkInfoForQuery to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENTS_URI, info.uri}
    };
}

void from_json(const nlohmann::json &jsonObject, PageInfoForQuery &pageInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "PageInfoForQuery from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_UI_ABILITY,
        pageInfo.uiAbility,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PAGE_PATH,
        pageInfo.pagePath,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAVIGATION_ID,
        pageInfo.navigationId,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAV_DESTINATION_NAME,
        pageInfo.navDestinationName,
        true,
        g_parseResult);
}

void to_json(nlohmann::json& jsonObject, const PageInfoForQuery &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "PageInfoForQuery to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_UI_ABILITY, info.uiAbility},
        {INSIGHT_INTENT_PAGE_PATH, info.pagePath},
        {INSIGHT_INTENT_NAVIGATION_ID, info.navigationId},
        {INSIGHT_INTENT_NAV_DESTINATION_NAME, info.navDestinationName}
    };
}

void from_json(const nlohmann::json &jsonObject, EntryInfoForQuery &entryInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "EntryInfoForQuery from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY_NAME,
        entryInfo.abilityName,
        true,
        g_parseResult);
    if (jsonObject.find(INSIGHT_INTENT_EXECUTE_MODE) != jsonObjectEnd) {
        const auto &modeArray = jsonObject[INSIGHT_INTENT_EXECUTE_MODE];
        for (const auto &modeStr : modeArray) {
            if (!modeStr.is_string()) {
                TAG_LOGE(AAFwkTag::INTENT, "modestr not string");
                continue;
            }
            std::string modeStrValue = modeStr.get<std::string>();
            auto it = STRING_EXECUTE_MODE_MAP.find(modeStrValue);
            if (it != STRING_EXECUTE_MODE_MAP.end()) {
                entryInfo.executeMode.push_back(it->second);
            } else {
                TAG_LOGW(AAFwkTag::INTENT, "Unknown ExecuteMode: %{public}s", modeStr.dump().c_str());
            }
        }
    }
}

void to_json(nlohmann::json& jsonObject, const EntryInfoForQuery &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "EntryInfoForQuery to json");
    std::vector<std::string> modeStrings;
    for (const auto &mode : info.executeMode) {
        auto it = EXECUTE_MODE_STRING_MAP.find(mode);
        if (it != EXECUTE_MODE_STRING_MAP.end()) {
            modeStrings.push_back(it->second);
        } else {
            modeStrings.push_back("UNKNOWN");
        }
    }
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_ABILITY_NAME, info.abilityName},
        {INSIGHT_INTENT_EXECUTE_MODE, modeStrings}
    };
}

void from_json(const nlohmann::json &jsonObject, FormInfoForQuery &formInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "FormInfoForQuery from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY_NAME,
        formInfo.abilityName,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FORM_NAME,
        formInfo.formName,
        true,
        g_parseResult);
}

void to_json(nlohmann::json& jsonObject, const FormInfoForQuery &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "FormInfoForQuery to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_ABILITY_NAME, info.abilityName},
        {INSIGHT_INTENT_FORM_NAME, info.formName}
    };
}

void from_json(const nlohmann::json &jsonObject, EntityInfoForQuery &entityInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "EntityInfoForQuery from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_CLASS_NAME,
        entityInfo.className,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_ID,
        entityInfo.entityId,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_CATEGORY,
        entityInfo.entityCategory,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_PARAMETERS,
        entityInfo.parameters,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_PARENT_CLASS_NAME,
        entityInfo.parentClassName,
        false,
        g_parseResult);
}

void to_json(nlohmann::json& jsonObject, const EntityInfoForQuery &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "EntityInfoForQuery to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_ENTITY_CLASS_NAME, info.className},
        {INSIGHT_INTENT_ENTITY_ID, info.entityId},
        {INSIGHT_INTENT_ENTITY_CATEGORY, info.entityCategory},
        {INSIGHT_INTENT_PARAMETERS, info.parameters},
        {INSIGHT_INTENT_ENTITY_PARENT_CLASS_NAME, info.parentClassName}
    };
}

void from_json(const nlohmann::json &jsonObject, InsightIntentInfoForQuery &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightIntentInfoForQuery from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_BUNDLE_NAME,
        insightIntentInfo.bundleName,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_MODULE_NAME,
        insightIntentInfo.moduleName,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_INTENT_NAME,
        insightIntentInfo.intentName,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DOMAIN,
        insightIntentInfo.domain,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_INTENT_VERSION,
        insightIntentInfo.intentVersion,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DISPLAY_NAME,
        insightIntentInfo.displayName,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DISPLAY_DESCRIPTION,
        insightIntentInfo.displayDescription,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_SCHEMA,
        insightIntentInfo.schema,
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
        INSIGHT_INTENT_LLM_DESCRIPTION,
        insightIntentInfo.llmDescription,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_INTENT_TYPE,
        insightIntentInfo.intentType,
        true,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAMETERS,
        insightIntentInfo.parameters,
        false,
        g_parseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_RESULT,
        insightIntentInfo.result,
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
    AppExecFwk::GetValueIfFindKey<std::vector<EntityInfoForQuery>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTITY_INFO,
        insightIntentInfo.entities,
        JsonType::ARRAY,
        false,
        g_parseResult,
        ArrayType::OBJECT);

    if (insightIntentInfo.intentType == INSIGHT_INTENTS_TYPE_LINK) {
        AppExecFwk::GetValueIfFindKey<LinkInfoForQuery>(jsonObject,
            jsonObjectEnd,
            INSIGHT_INTENT_LINK_INFO,
            insightIntentInfo.linkInfo,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    } else if (insightIntentInfo.intentType == INSIGHT_INTENTS_TYPE_PAGE) {
        AppExecFwk::GetValueIfFindKey<PageInfoForQuery>(jsonObject,
            jsonObjectEnd,
            INSIGHT_INTENT_PAGE_INFO,
            insightIntentInfo.pageInfo,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    } else if (insightIntentInfo.intentType == INSIGHT_INTENTS_TYPE_ENTRY) {
        AppExecFwk::GetValueIfFindKey<EntryInfoForQuery>(jsonObject,
            jsonObjectEnd,
            INSIGHT_INTENT_ENTRY_INFO,
            insightIntentInfo.entryInfo,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    } else if (insightIntentInfo.intentType == INSIGHT_INTENTS_TYPE_FORM) {
        AppExecFwk::GetValueIfFindKey<FormInfoForQuery>(jsonObject,
            jsonObjectEnd,
            INSIGHT_INTENT_FORM_INFO,
            insightIntentInfo.formInfo,
            JsonType::OBJECT,
            false,
            g_parseResult,
            ArrayType::NOT_ARRAY);
    }
}

void to_json(nlohmann::json& jsonObject, const InsightIntentInfoForQuery &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightIntentInfoForQuery to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_BUNDLE_NAME, info.bundleName},
        {INSIGHT_INTENT_MODULE_NAME, info.moduleName},
        {INSIGHT_INTENT_INTENT_NAME, info.intentName},
        {INSIGHT_INTENT_DOMAIN, info.domain},
        {INSIGHT_INTENT_INTENT_VERSION, info.intentVersion},
        {INSIGHT_INTENT_DISPLAY_NAME, info.displayName},
        {INSIGHT_INTENT_DISPLAY_DESCRIPTION, info.displayDescription},
        {INSIGHT_INTENT_SCHEMA, info.schema},
        {INSIGHT_INTENT_ICON, info.icon},
        {INSIGHT_INTENT_LLM_DESCRIPTION, info.llmDescription},
        {INSIGHT_INTENT_INTENT_TYPE, info.intentType},
        {INSIGHT_INTENT_PARAMETERS, info.parameters},
        {INSIGHT_INTENT_RESULT, info.result},
        {INSIGHT_INTENT_KEYWORDS, info.keywords},
        {INSIGHT_INTENT_LINK_INFO, info.linkInfo},
        {INSIGHT_INTENT_PAGE_INFO, info.pageInfo},
        {INSIGHT_INTENT_ENTRY_INFO, info.entryInfo},
        {INSIGHT_INTENT_FORM_INFO, info.formInfo},
        {INSIGHT_INTENT_ENTITY_INFO, info.entities}
    };
}

bool InsightIntentInfoForQuery::ReadFromParcel(Parcel &parcel)
{
    MessageParcel *messageParcel = reinterpret_cast<MessageParcel *>(&parcel);
    if (!messageParcel) {
        TAG_LOGE(AAFwkTag::INTENT, "Type conversion failed");
        return false;
    }
    uint32_t length = messageParcel->ReadUint32();
    if (length == 0) {
        TAG_LOGE(AAFwkTag::INTENT, "Invalid data length");
        return false;
    }
    const char *data = reinterpret_cast<const char *>(messageParcel->ReadRawData(length));
    TAG_LOGD(AAFwkTag::INTENT, "ReadFromParcel data: %{public}s", data);
    if (!data) {
        TAG_LOGE(AAFwkTag::INTENT, "Fail read raw length = %{public}d", length);
        return false;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(data, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "failed to parse BundleInfo");
        return false;
    }
    std::lock_guard<std::mutex> lock(g_extraMutex);
    g_parseResult = ERR_OK;
    *this = jsonObject.get<InsightIntentInfoForQuery>();
    if (g_parseResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "parse result: %{public}d", g_parseResult);
        g_parseResult = ERR_OK;
        return false;
    }
    return true;
}

bool InsightIntentInfoForQuery::Marshalling(Parcel &parcel) const
{
    MessageParcel *messageParcel = reinterpret_cast<MessageParcel *>(&parcel);
    if (!messageParcel) {
        TAG_LOGE(AAFwkTag::INTENT, "Conversion failed");
        return false;
    }
    nlohmann::json jsonObject = *this;
    std::string str = jsonObject.dump();
    TAG_LOGD(AAFwkTag::INTENT, "Marshalling str: %{public}s", str.c_str());
    if (!messageParcel->WriteUint32(str.size() + 1)) {
        TAG_LOGE(AAFwkTag::INTENT, "Write intent info size failed");
        return false;
    }
    if (!messageParcel->WriteRawData(str.c_str(), str.size() + 1)) {
        TAG_LOGE(AAFwkTag::INTENT, "Write intent info failed");
        return false;
    }
    return true;
}

InsightIntentInfoForQuery *InsightIntentInfoForQuery::Unmarshalling(Parcel &parcel)
{
    InsightIntentInfoForQuery *info = new (std::nothrow) InsightIntentInfoForQuery();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}
} // namespace AbilityRuntime
} // namespace OHOS
