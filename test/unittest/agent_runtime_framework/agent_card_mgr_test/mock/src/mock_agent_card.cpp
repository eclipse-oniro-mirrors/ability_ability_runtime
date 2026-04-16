/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "mock_my_flag.h"

namespace OHOS {
namespace AgentRuntime {
bool AgentProvider::ReadFromParcel(Parcel &parcel)
{
    return true;
}

bool AgentProvider::Marshalling(Parcel &parcel) const
{
    return true;
}

AgentProvider *AgentProvider::Unmarshalling(Parcel &parcel)
{
    return nullptr;
}

nlohmann::json AgentProvider::ToJson()
{
    return nlohmann::json();
}

bool AgentProvider::FromJson(const nlohmann::json &jsonObject, AgentProvider &provider)
{
    return true;
}

bool AgentCapabilities::ReadFromParcel(Parcel &parcel)
{
    return true;
}

bool AgentCapabilities::Marshalling(Parcel &parcel) const
{
    return true;
}

AgentCapabilities *AgentCapabilities::Unmarshalling(Parcel &parcel)
{
    return nullptr;
}

nlohmann::json AgentCapabilities::ToJson()
{
    return nlohmann::json();
}

AgentCapabilities AgentCapabilities::FromJson(const nlohmann::json &jsonObject)
{
    return AgentCapabilities();
}

bool AgentSkill::ReadFromParcel(Parcel &parcel)
{
    return true;
}

bool AgentSkill::Marshalling(Parcel &parcel) const
{
    return true;
}

AgentSkill *AgentSkill::Unmarshalling(Parcel &parcel)
{
    return nullptr;
}

nlohmann::json AgentSkill::ToJson()
{
    return nlohmann::json();
}

bool AgentSkill::FromJson(const nlohmann::json &jsonObject, AgentSkill &skill)
{
    return true;
}

bool AgentAppInfo::ReadFromParcel(Parcel &parcel)
{
    return true;
}

bool AgentAppInfo::Marshalling(Parcel &parcel) const
{
    return true;
}

AgentAppInfo *AgentAppInfo::Unmarshalling(Parcel &parcel)
{
    return nullptr;
}

nlohmann::json AgentAppInfo::ToJson()
{
    return nlohmann::json();
}

bool AgentAppInfo::FromJson(const nlohmann::json &jsonObject, AgentAppInfo &appInfo)
{
    return true;
}

bool AgentCard::ReadFromParcel(Parcel &parcel)
{
    return true;
}

bool AgentCard::Marshalling(Parcel &parcel) const
{
    return true;
}

AgentCard *AgentCard::Unmarshalling(Parcel &parcel)
{
    return nullptr;
}

nlohmann::json AgentCard::ToJson() const
{
    return nlohmann::json();
}

bool AgentCard::FromJson(nlohmann::json jsonObject, AgentCard &agentCard)
{
    if (!MyFlag::retFromJson) {
        return false;
    }
    if (jsonObject.contains("type")) {
        if (jsonObject["type"].is_number_integer()) {
            agentCard.type = static_cast<AgentCardType>(jsonObject["type"].get<int32_t>());
        } else if (jsonObject["type"].is_string()) {
            std::string typeName = jsonObject["type"].get<std::string>();
            if (typeName == "APP") {
                agentCard.type = AgentCardType::APP;
            } else if (typeName == "LOW_CODE") {
                agentCard.type = AgentCardType::LOW_CODE;
            } else if (typeName == "ATOMIC_SERVICE") {
                agentCard.type = AgentCardType::ATOMIC_SERVICE;
            }
        }
    }
    if (jsonObject.contains("agentId") && jsonObject["agentId"].is_string()) {
        agentCard.agentId = jsonObject["agentId"].get<std::string>();
    }
    if (jsonObject.contains("name") && jsonObject["name"].is_string()) {
        agentCard.name = jsonObject["name"].get<std::string>();
    }
    if (jsonObject.contains("description") && jsonObject["description"].is_string()) {
        agentCard.description = jsonObject["description"].get<std::string>();
    }
    if (jsonObject.contains("version") && jsonObject["version"].is_string()) {
        agentCard.version = jsonObject["version"].get<std::string>();
    }
    if (jsonObject.contains("category") && jsonObject["category"].is_string()) {
        agentCard.category = jsonObject["category"].get<std::string>();
    }
    if (jsonObject.contains("defaultInputModes") && jsonObject["defaultInputModes"].is_array()) {
        agentCard.defaultInputModes = jsonObject["defaultInputModes"].get<std::vector<std::string>>();
    }
    if (jsonObject.contains("defaultOutputModes") && jsonObject["defaultOutputModes"].is_array()) {
        agentCard.defaultOutputModes = jsonObject["defaultOutputModes"].get<std::vector<std::string>>();
    }
    if (jsonObject.contains("appInfo") && jsonObject["appInfo"].is_object()) {
        agentCard.appInfo = std::make_shared<AgentAppInfo>();
        auto appInfoJson = jsonObject["appInfo"];
        if (appInfoJson.contains("deviceTypes") && appInfoJson["deviceTypes"].is_array()) {
            agentCard.appInfo->deviceTypes = appInfoJson["deviceTypes"].get<std::vector<std::string>>();
        }
    }
    return true;
}

void AgentCardsRawData::FromAgentCardVec(const std::vector<AgentCard> &cards, AgentCardsRawData &rawData)
{
    rawData.size = cards.size();
}

int32_t AgentCardsRawData::ToAgentCardVec(const AgentCardsRawData &rawData, std::vector<AgentCard> &cards)
{
    return 0;
}

int32_t AgentCardsRawData::RawDataCpy(const void *readdata)
{
    return 0;
}

AgentCardsRawData::~AgentCardsRawData()
{}
} // namespace AgentRuntime
} // namespace OHOS
