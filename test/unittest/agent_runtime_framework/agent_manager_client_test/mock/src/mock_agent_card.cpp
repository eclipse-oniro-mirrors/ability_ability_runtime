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
int MyFlag::retToAgentCardVec = 0;
std::vector<AgentCard> MyFlag::convertedCards;

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
    AgentProvider *provider = new (std::nothrow) AgentProvider();
    return provider;
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
    AgentCapabilities *capabilities = new (std::nothrow) AgentCapabilities();
    return capabilities;
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
    AgentSkill *skill = new (std::nothrow) AgentSkill();
    return skill;
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
    AgentCard *agentCard = new (std::nothrow) AgentCard();
    return agentCard;
}

nlohmann::json AgentCard::ToJson() const
{
    nlohmann::json jsonObject;
    return jsonObject;
}

bool AgentCard::FromJson(nlohmann::json jsonObject, AgentCard &agentCard)
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
    AgentAppInfo *appInfo = new (std::nothrow) AgentAppInfo();
    return appInfo;
}

nlohmann::json AgentAppInfo::ToJson()
{
    nlohmann::json jsonObject;
    return jsonObject;
}

bool AgentAppInfo::FromJson(const nlohmann::json &jsonObject, AgentAppInfo &appInfo)
{
    return true;
}

int32_t AgentCardsRawData::RawDataCpy(const void *readdata)
{
    return 0;
}

int32_t AgentCardsRawData::ToAgentCardVec(const AgentCardsRawData &rawData, std::vector<AgentCard> &cards)
{
    cards = MyFlag::convertedCards;
    return MyFlag::retToAgentCardVec;
}

AgentCardsRawData::~AgentCardsRawData()
{
}

void AgentCardsRawData::FromAgentCardVec(const std::vector<AgentCard> &cards, AgentCardsRawData &rawData)
{
}
} // namespace AgentRuntime
} // namespace OHOS