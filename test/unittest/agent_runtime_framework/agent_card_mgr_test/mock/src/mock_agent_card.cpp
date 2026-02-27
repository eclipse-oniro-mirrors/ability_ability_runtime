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
    return MyFlag::retFromJson;
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
