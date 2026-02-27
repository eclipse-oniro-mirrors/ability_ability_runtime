/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "agent_card.h"

#include <map>
#include <sstream>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "securec.h"
#include "string_ex.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
constexpr int32_t SKILLS_MAX_SIZE = 100;
constexpr uint32_t MAX_AGENT_CARD_COUNT = 200000;
constexpr uint32_t LENGTH_32 = 32;
constexpr uint32_t LENGTH_64 = 64;
constexpr uint32_t LENGTH_128 = 128;
constexpr uint32_t LENGTH_256 = 256;
constexpr uint32_t LENGTH_512 = 512;
constexpr uint32_t LENGTH_1024 = 1024;
constexpr uint32_t LENGTH_5120 = 5120;

bool AgentProvider::ReadFromParcel(Parcel &parcel)
{
    organization = parcel.ReadString();
    url = parcel.ReadString();
    return true;
}

bool AgentProvider::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(organization)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write organization failed");
        return false;
    }
    if (!parcel.WriteString(url)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write url failed");
        return false;
    }
    return true;
}

AgentProvider *AgentProvider::Unmarshalling(Parcel &parcel)
{
    AgentProvider *provider = new (std::nothrow) AgentProvider();
    if (provider && !provider->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "provider unmarshalling failed");
        delete provider;
        provider = nullptr;
    }
    return provider;
}

nlohmann::json AgentProvider::ToJson()
{
    return nlohmann::json {
        { "organization", organization },
        { "url", url },
    };
}

bool AgentProvider::FromJson(const nlohmann::json &jsonObject, AgentProvider &provider)
{
    if (!jsonObject.contains("organization") || !jsonObject["organization"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, organization not exist");
        return false;
    }
    provider.organization = jsonObject["organization"];
    if (provider.organization.length() > LENGTH_128 || provider.organization.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "organization is long than 128 or empty");
        return false;
    }
    if (!jsonObject.contains("url") || !jsonObject["url"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, url not exist");
        return false;
    }
    provider.url = jsonObject["url"];
    if (provider.url.length() > LENGTH_512 || provider.url.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "url is long than 512 or empty");
        return false;
    }
    return true;
}

bool AgentCapabilities::ReadFromParcel(Parcel &parcel)
{
    streaming = parcel.ReadBool();
    pushNotifications = parcel.ReadBool();
    stateTransitionHistory = parcel.ReadBool();
    extension = parcel.ReadString();
    extendedAgentCard = parcel.ReadBool();
    return true;
}

bool AgentCapabilities::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(streaming)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write streaming failed");
        return false;
    }
    if (!parcel.WriteBool(pushNotifications)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write pushNotifications failed");
        return false;
    }
    if (!parcel.WriteBool(stateTransitionHistory)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write stateTransitionHistory failed");
        return false;
    }
    if (!parcel.WriteString(extension)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write extension failed");
        return false;
    }
    if (!parcel.WriteBool(extendedAgentCard)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write extendedAgentCard failed");
        return false;
    }
    return true;
}

AgentCapabilities *AgentCapabilities::Unmarshalling(Parcel &parcel)
{
    AgentCapabilities *capabilities = new (std::nothrow) AgentCapabilities();
    if (capabilities && !capabilities->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "capabilities unmarshalling failed");
        delete capabilities;
        capabilities = nullptr;
    }
    return capabilities;
}

nlohmann::json AgentCapabilities::ToJson()
{
    nlohmann::json jsonObject;
    jsonObject["streaming"] = streaming;
    jsonObject["pushNotifications"] = pushNotifications;
    jsonObject["stateTransitionHistory"] = stateTransitionHistory;
    jsonObject["extension"] = extension;
    jsonObject["extendedAgentCard"] = extendedAgentCard;
    return jsonObject;
}

AgentCapabilities AgentCapabilities::FromJson(const nlohmann::json &jsonObject)
{
    AgentCapabilities capabilities;
    if (jsonObject.contains("streaming") && jsonObject["streaming"].is_boolean()) {
        capabilities.streaming = jsonObject.at("streaming");
    }
    if (jsonObject.contains("pushNotifications") && jsonObject["pushNotifications"].is_boolean()) {
        capabilities.pushNotifications = jsonObject.at("pushNotifications");
    }
    if (jsonObject.contains("stateTransitionHistory") && jsonObject["stateTransitionHistory"].is_boolean()) {
        capabilities.stateTransitionHistory = jsonObject.at("stateTransitionHistory");
    }
    if (jsonObject.contains("extension") && jsonObject["extension"].is_string()) {
        capabilities.extension = jsonObject["extension"];
        if (capabilities.extension.length() < 1 || capabilities.extension.length() > LENGTH_1024) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "extension length is invalid");
            capabilities.extension = "";
        }
    }
    if (jsonObject.contains("extendedAgentCard") && jsonObject["extendedAgentCard"].is_boolean()) {
        capabilities.extendedAgentCard = jsonObject.at("extendedAgentCard");
    }
    return capabilities;
}

bool AgentSkill::ReadFromParcel(Parcel &parcel)
{
    id = parcel.ReadString();
    name = parcel.ReadString();
    description = parcel.ReadString();
    if (!parcel.ReadStringVector(&tags)) {
        return false;
    }
    if (!parcel.ReadStringVector(&examples)) {
        return false;
    }
    if (!parcel.ReadStringVector(&inputModes)) {
        return false;
    }
    if (!parcel.ReadStringVector(&outputModes)) {
        return false;
    }
    extension = parcel.ReadString();
    return true;
}

bool AgentSkill::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(id)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write id failed");
        return false;
    }
    if (!parcel.WriteString(name)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write name failed");
        return false;
    }
    if (!parcel.WriteString(description)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write description failed");
        return false;
    }
    if (!parcel.WriteStringVector(tags)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write tags failed");
        return false;
    }
    if (!parcel.WriteStringVector(examples)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write examples failed");
        return false;
    }
    if (!parcel.WriteStringVector(inputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write inputModes failed");
        return false;
    }
    if (!parcel.WriteStringVector(outputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write outputModes failed");
        return false;
    }
    if (!parcel.WriteString(extension)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write extension failed");
        return false;
    }
    return true;
}

AgentSkill *AgentSkill::Unmarshalling(Parcel &parcel)
{
    AgentSkill *skill = new (std::nothrow) AgentSkill();
    if (skill && !skill->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "skill unmarshalling failed");
        delete skill;
        skill = nullptr;
    }
    return skill;
}

nlohmann::json AgentSkill::ToJson()
{
    return nlohmann::json {
        { "id", id },
        { "name", name },
        { "description", description },
        { "tags", tags},
        { "examples", examples},
        { "inputModes", inputModes},
        { "outputModes", outputModes},
        { "extension", extension},
    };
}

bool AgentSkill::FromJson(const nlohmann::json &jsonObject, AgentSkill &skill)
{
    if (!jsonObject.contains("id") || !jsonObject["id"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, id not exist");
        return false;
    }
    skill.id = jsonObject["id"];
    if (skill.id.length() > LENGTH_64 || skill.id.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "id is long than 64 or empty");
        return false;
    }
    if (!jsonObject.contains("name") || !jsonObject["name"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, name not exist");
        return false;
    }
    skill.name = jsonObject["name"];
    if (skill.name.length() > LENGTH_128 || skill.name.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "name is long than 128 or empty");
        return false;
    }
    if (!jsonObject.contains("description") || !jsonObject["description"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, description not exist");
        return false;
    }
    skill.description = jsonObject["description"];
    if (skill.description.length() > LENGTH_512 || skill.description.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "description is long than 512 or empty");
        return false;
    }
    if (!jsonObject.contains("tags") || !jsonObject["tags"].is_array()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, tags not exist");
        return false;
    }
    for (const auto &element : jsonObject.at("tags")) {
        if (element.is_string()) {
            std::string tag = element.get<std::string>();
            if (tag.length() >= 1 && tag.length() <= LENGTH_32) {
                skill.tags.push_back(tag);
            }
        }
    }
    if (skill.tags.size() == 0 || skill.tags.size() > SKILLS_MAX_SIZE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "tags size is 0 or exceeds max");
        return false;
    }
    if (jsonObject.contains("examples") && jsonObject["examples"].is_array()) {
        for (const auto &element : jsonObject.at("examples")) {
            if (!element.is_string()) {
                continue;
            }
            std::string exampleStr = element.get<std::string>();
            if (exampleStr.length() <= LENGTH_256 && exampleStr.length() > 0) {
                skill.examples.push_back(exampleStr);
            }
        }
    }
    if (jsonObject.contains("inputModes") && jsonObject["inputModes"].is_array()) {
        for (const auto &element : jsonObject.at("inputModes")) {
            if (!element.is_string()) {
                continue;
            }
            std::string inputModeStr = element.get<std::string>();
            if (inputModeStr.length() <= LENGTH_32 && inputModeStr.length() > 0) {
                skill.inputModes.push_back(inputModeStr);
            }
        }
    }
    if (jsonObject.contains("outputModes") && jsonObject["outputModes"].is_array()) {
        for (const auto &element : jsonObject.at("outputModes")) {
            if (!element.is_string()) {
                continue;
            }
            std::string outputModeStr = element.get<std::string>();
            if (outputModeStr.length() <= LENGTH_32 && outputModeStr.length() > 0) {
                skill.outputModes.push_back(outputModeStr);
            }
        }
    }
    if (jsonObject.contains("extension") && jsonObject["extension"].is_string()) {
        skill.extension = jsonObject["extension"];
        if (skill.extension.length() < 1 || skill.extension.length() > LENGTH_1024) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "extension length is invalid");
            skill.extension = "";
        }
    }
    return true;
}

bool AgentAppInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    moduleName = parcel.ReadString();
    abilityName = parcel.ReadString();
    if (!parcel.ReadStringVector(&deviceTypes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "read deviceTypes failed");
        return false;
    }
    minAppVersion = parcel.ReadString();
    return true;
}

bool AgentAppInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write bundleName failed");
        return false;
    }
    if (!parcel.WriteString(moduleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write moduleName failed");
        return false;
    }
    if (!parcel.WriteString(abilityName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write abilityName failed");
        return false;
    }
    if (!parcel.WriteStringVector(deviceTypes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write deviceTypes failed");
        return false;
    }
    if (!parcel.WriteString(minAppVersion)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write minAppVersion failed");
        return false;
    }
    return true;
}

AgentAppInfo *AgentAppInfo::Unmarshalling(Parcel &parcel)
{
    AgentAppInfo *appInfo = new (std::nothrow) AgentAppInfo();
    if (appInfo && !appInfo->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "appInfo unmarshalling failed");
        delete appInfo;
        appInfo = nullptr;
    }
    return appInfo;
}

nlohmann::json AgentAppInfo::ToJson()
{
    return nlohmann::json {
        { "bundleName", bundleName },
        { "moduleName", moduleName },
        { "abilityName", abilityName },
        { "deviceTypes", deviceTypes },
        { "minAppVersion", minAppVersion },
    };
}

bool AgentAppInfo::FromJson(const nlohmann::json &jsonObject, AgentAppInfo &appInfo)
{
    if (jsonObject.contains("bundleName") && jsonObject["bundleName"].is_string()) {
        appInfo.bundleName = jsonObject["bundleName"];
    }
    if (jsonObject.contains("moduleName") && jsonObject["moduleName"].is_string()) {
        appInfo.moduleName = jsonObject["moduleName"];
    }
    if (jsonObject.contains("abilityName") && jsonObject["abilityName"].is_string()) {
        appInfo.abilityName = jsonObject["abilityName"];
    }
    if (jsonObject.contains("deviceTypes") && jsonObject["deviceTypes"].is_array()) {
        for (const auto &element : jsonObject.at("deviceTypes")) {
            if (!element.is_string()) {
                continue;
            }
            std::string deviceType = element.get<std::string>();
            if (deviceType.length() > 0 && deviceType.length() <= LENGTH_32) {
                appInfo.deviceTypes.push_back(deviceType);
            }
        }
    }
    if (jsonObject.contains("minAppVersion") && jsonObject["minAppVersion"].is_string()) {
        appInfo.minAppVersion = jsonObject["minAppVersion"];
        if (appInfo.minAppVersion.length() < 1 || appInfo.minAppVersion.length() > LENGTH_32) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "minAppVersion length is invalid");
            appInfo.minAppVersion = "";
        }
    }
    return true;
}

bool AgentCard::ReadFromParcel(Parcel &parcel)
{
    agentId = parcel.ReadString();
    name = parcel.ReadString();
    description = parcel.ReadString();
    provider.reset(parcel.ReadParcelable<AgentProvider>());
    version = parcel.ReadString();
    documentationUrl = parcel.ReadString();
    category = parcel.ReadString();
    capabilities.reset(parcel.ReadParcelable<AgentCapabilities>());
    if (!parcel.ReadStringVector(&defaultInputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "read defaultInputModes failed");
        return false;
    }
    if (!parcel.ReadStringVector(&defaultOutputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "read defaultOutputModes failed");
        return false;
    }
    uint32_t skillSize = parcel.ReadUint32();
    if (skillSize > SKILLS_MAX_SIZE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "read skill size failed");
        return false;
    }
    for (uint32_t i = 0; i < skillSize; i++) {
        std::shared_ptr<AgentSkill> skill(parcel.ReadParcelable<AgentSkill>());
        skills.push_back(skill);
    }
    iconUrl = parcel.ReadString();
    extension = parcel.ReadString();
    appInfo.reset(parcel.ReadParcelable<AgentAppInfo>());
    return true;
}

bool AgentCard::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write agentId failed");
        return false;
    }
    if (!parcel.WriteString(name)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write name failed");
        return false;
    }
    if (!parcel.WriteString(description)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write description failed");
        return false;
    }
    if (!parcel.WriteParcelable(provider.get())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write provider failed.");
        return false;
    }
    if (!parcel.WriteString(version)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write version failed");
        return false;
    }
    if (!parcel.WriteString(documentationUrl)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write documentationUrl failed");
        return false;
    }
    if (!parcel.WriteString(category)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write category failed");
        return false;
    }
    if (!parcel.WriteParcelable(capabilities.get())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write capabilities failed.");
        return false;
    }
    if (!parcel.WriteStringVector(defaultInputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write defaultInputModes failed");
        return false;
    }
    if (!parcel.WriteStringVector(defaultOutputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write defaultOutputModes failed");
        return false;
    }
    if (!parcel.WriteUint32(skills.size())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Write skill size failed.");
        return false;
    }
    for (auto skill : skills) {
        if (!parcel.WriteParcelable(skill.get())) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Write skill failed.");
            return false;
        }
    }
    if (!parcel.WriteString(iconUrl)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write iconUrl failed");
        return false;
    }
    if (!parcel.WriteString(extension)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write extension failed");
        return false;
    }
    if (!parcel.WriteParcelable(appInfo.get())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write appInfo failed");
        return false;
    }
    return true;
}

AgentCard *AgentCard::Unmarshalling(Parcel &parcel)
{
    AgentCard *agentCard = new (std::nothrow) AgentCard();
    if (agentCard && !agentCard->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agentCard unmarshalling failed");
        delete agentCard;
        agentCard = nullptr;
    }
    return agentCard;
}

nlohmann::json AgentCard::ToJson() const
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", agentId },
        { "name", name },
        { "description", description },
        { "version", version},
        { "documentationUrl", documentationUrl},
        { "defaultInputModes", defaultInputModes},
        { "defaultOutputModes", defaultOutputModes},
    };

    // Add optional fields if they are not empty/null
    if (provider != nullptr) {
        jsonObject["provider"] = provider->ToJson();
    }

    if (capabilities != nullptr) {
        jsonObject["capabilities"] = capabilities->ToJson();
    }

    if (!skills.empty()) {
        nlohmann::json skillsArray = nlohmann::json::array();
        for (const auto& skill : skills) {
            if (skill != nullptr) {
                nlohmann::json skillJson = skill->ToJson();
                skillsArray.push_back(skillJson);
            }
        }
        jsonObject["skills"] = skillsArray;
    }
    if (!iconUrl.empty()) {
        jsonObject["iconUrl"] = iconUrl;
    }

    if (!extension.empty()) {
        jsonObject["extension"] = extension;
    }

    // Required category field
    if (!category.empty()) {
        jsonObject["category"] = category;
    }

    // Optional appInfo field
    if (appInfo != nullptr) {
        jsonObject["appInfo"] = appInfo->ToJson();
    }

    return jsonObject;
}

bool AgentCard::FromJson(nlohmann::json jsonObject, AgentCard &agentCard)
{
    // Required fields
    if (!jsonObject.contains("agentId") || !jsonObject["agentId"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, agentId not exist");
        return false;
    }
    agentCard.agentId = jsonObject["agentId"];
    if (agentCard.agentId.length() > LENGTH_64 || agentCard.agentId.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agentId is long than 64 or empty");
        return false;
    }
    if (!jsonObject.contains("name") || !jsonObject["name"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, name not exist");
        return false;
    }
    agentCard.name = jsonObject["name"];
    if (agentCard.name.length() > LENGTH_64 || agentCard.name.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "name is long than 64 or empty");
        return false;
    }
    // Required category field
    if (!jsonObject.contains("category") || !jsonObject["category"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, category not exist");
        return false;
    }
    agentCard.category = jsonObject["category"];
    if (agentCard.category.length() < 1 || agentCard.category.length() > LENGTH_64) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "category is invalid");
        return false;
    }
    if (jsonObject.contains("appInfo") && jsonObject["appInfo"].is_object()) {
        agentCard.appInfo = std::make_shared<AgentAppInfo>();
        if (!AgentAppInfo::FromJson(jsonObject["appInfo"], *agentCard.appInfo)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "appInfo FromJson failed");
            agentCard.appInfo = nullptr;
            return false;
        }
    }
    if (!jsonObject.contains("description") || !jsonObject["description"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, description not exist");
        return false;
    }
    agentCard.description = jsonObject["description"];
    if (agentCard.description.length() > LENGTH_512 || agentCard.description.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "description is long than 512 or empty");
        return false;
    }
    if (!jsonObject.contains("version") || !jsonObject["version"].is_string()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, version not exist");
        return false;
    }
    agentCard.version = jsonObject["version"];
    if (agentCard.version.length() > LENGTH_32 || agentCard.version.length() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "version is long than 32 or empty");
        return false;
    }
    if (jsonObject.contains("documentationUrl") && jsonObject["documentationUrl"].is_string()) {
        agentCard.documentationUrl = jsonObject["documentationUrl"];
        if (agentCard.documentationUrl.length() > LENGTH_512) {
            agentCard.documentationUrl = "";
        }
    }

    if (!jsonObject.contains("defaultInputModes") || !jsonObject["defaultInputModes"].is_array()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "defaultInputModes is empty");
        return false;
    }
    for (const auto &element : jsonObject.at("defaultInputModes")) {
        if (!element.is_string()) {
            continue;
        }
        std::string intputMode = element.get<std::string>();
        if (intputMode.length() >= 1 && intputMode.length() <= LENGTH_32) {
            agentCard.defaultInputModes.push_back(intputMode);
        }
    }
    if (agentCard.defaultInputModes.size() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "defaultInputModes is empty");
        return false;
    }

    if (!jsonObject.contains("defaultOutputModes") || !jsonObject["defaultOutputModes"].is_array()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "defaultOutputModes is empty");
        return false;
    }
    for (const auto &element : jsonObject.at("defaultOutputModes")) {
        if (!element.is_string()) {
            continue;
        }
        std::string outputMode = element.get<std::string>();
        if (outputMode.length() >= 1 && outputMode.length() <= LENGTH_32) {
            agentCard.defaultOutputModes.push_back(outputMode);
        }
    }
    if (agentCard.defaultOutputModes.size() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "defaultInputModes is empty");
        return false;
    }

    // Optional objects
    if (jsonObject.contains("provider") && jsonObject["provider"].is_object()) {
        auto provider = std::make_shared<AgentProvider>();
        if (AgentProvider::FromJson(jsonObject["provider"], *provider)) {
            agentCard.provider = provider;
        }
    }

    if (jsonObject.contains("capabilities") && jsonObject["capabilities"].is_object()) {
        auto capabilities = std::make_shared<AgentCapabilities>();
        *capabilities = AgentCapabilities::FromJson(jsonObject["capabilities"]);
        agentCard.capabilities = capabilities;
    }

    // Skills array
    if (jsonObject.contains("skills") && jsonObject["skills"].is_array()) {
        std::map<std::string, std::shared_ptr<AgentSkill>> skillMap;
        for (const auto& skillJson : jsonObject["skills"]) {
            if (skillJson.is_object()) {
                auto skill = std::make_shared<AgentSkill>();
                if (AgentSkill::FromJson(skillJson, *skill)) {
                    // Later skills with same id will overwrite earlier ones
                    skillMap[skill->id] = skill;
                }
            }
        }
        // Convert map to vector
        for (const auto& [id, skill] : skillMap) {
            agentCard.skills.push_back(skill);
        }
    }
    if (agentCard.skills.size() == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "skills is empty");
        return false;
    }

    // Optional iconUrl field
    if (jsonObject.contains("iconUrl") && jsonObject["iconUrl"].is_string()) {
        agentCard.iconUrl = jsonObject["iconUrl"];
        if (agentCard.iconUrl.length() < 1 || agentCard.iconUrl.length() > LENGTH_512) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "iconUrl length is invalid");
            agentCard.iconUrl = "";
        }
    }

    // Optional extension field
    if (jsonObject.contains("extension") && jsonObject["extension"].is_string()) {
        agentCard.extension = jsonObject["extension"];
        if (agentCard.extension.length() < 1 || agentCard.extension.length() > LENGTH_5120) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "extension length is invalid");
            agentCard.extension = "";
        }
    }

    return true;
}

int32_t AgentCardsRawData::RawDataCpy(const void *readdata)
{
    if (readdata == nullptr || size == 0) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null data or zero size");
        return ERR_INVALID_AGENT_CARD_DATA;
    }
    void* newData = malloc(size);
    if (newData == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "malloc failed");
        return ERR_INVALID_AGENT_CARD_DATA;
    }
    if (memcpy_s(newData, size, readdata, size) != EOK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "memcpy_s failed");
        free(newData);
        return ERR_INVALID_AGENT_CARD_DATA;
    }
    isMalloc = true;
    if (data != nullptr) {
        free(const_cast<void*>(data));
        data = nullptr;
    }
    data = newData;
    return ERR_OK;
}

int32_t AgentCardsRawData::ToAgentCardVec(const AgentCardsRawData &rawData, std::vector<AgentCard> &cards)
{
    std::stringstream ss;
    ss.write(reinterpret_cast<const char *>(rawData.data), rawData.size);
    ss.seekg(0, std::ios::beg);
    uint32_t ssLength = static_cast<uint32_t>(ss.str().length());
    uint32_t count = 0;
    ss.read(reinterpret_cast<char *>(&count), sizeof(count));
    if (count > MAX_AGENT_CARD_COUNT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "cards exceed maxSize %{public}d, count: %{public}d",
            MAX_AGENT_CARD_COUNT, count);
        return ERR_AGENT_CARD_LIST_OUT_OF_RANGE;
    }
    cards.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t cardSize = 0;
        ss.read(reinterpret_cast<char *>(&cardSize), sizeof(cardSize));
        if (cardSize > ssLength - static_cast<uint32_t>(ss.tellg())) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "cardSize:%{public}u is invalid", cardSize);
            return ERR_INVALID_AGENT_CARD_DATA;
        }
        std::string cardStr(cardSize, '\0');
        ss.read(cardStr.data(), cardSize);
        nlohmann::json jsonObject = nlohmann::json::parse(cardStr, nullptr, false);
        if (!AgentCard::FromJson(jsonObject, cards[i])) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed, index: %{public}u", i);
            return ERR_INVALID_AGENT_CARD_DATA;
        }
    }
    return ERR_OK;
}

AgentCardsRawData::~AgentCardsRawData()
{
    if (data != nullptr && isMalloc) {
        free(const_cast<void*>(data));
        isMalloc = false;
        data = nullptr;
    }
}

void AgentCardsRawData::FromAgentCardVec(const std::vector<AgentCard> &cards, AgentCardsRawData &rawData)
{
    std::stringstream ss;
    uint32_t count = cards.size();
    ss.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (uint32_t i = 0; i < count; ++i) {
        std::string dumped = cards[i].ToJson().dump();
        uint32_t strLen = dumped.length();
        ss.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
        ss.write(dumped.c_str(), strLen);
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
}
} // namespace AgentRuntime
} // namespace OHOS