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

#include "agent_card_mgr.h"

#include <algorithm>
#include <unistd.h>
#include <unordered_map>

#include "ability_manager_errors.h"
#include "agent_card.h"
#include "agent_card_db_mgr.h"
#include "agent_card_utils.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "json_utils.h"
#include "sem_ver.h"

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AppExecFwk;
using json = nlohmann::json;
namespace {
constexpr const char* AGENT_CONFIG = "ohos.extension.agent";
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr int32_t MAX_AGENT_CARD_SIZE = 1000;
constexpr int32_t GET_AGENT_EXTENSION_BUNDLE_INFO_FLAGS =
    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) |
    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) |
    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA) |
    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);

std::vector<AgentCard> ExtractCards(const std::vector<StoredAgentCardEntry> &entries)
{
    std::vector<AgentCard> cards;
    cards.reserve(entries.size());
    for (const auto &entry : entries) {
        cards.emplace_back(entry.card);
    }
    return cards;
}

bool ShouldKeepStoredBundleEntry(const AgentCard &incomingCard, const StoredAgentCardEntry &storedEntry)
{
    if (storedEntry.source == AgentCardUpdateSource::API && IsValidSemVer(incomingCard.version) &&
        IsValidSemVer(storedEntry.card.version) &&
        CompareSemVer(incomingCard.version, storedEntry.card.version) == SemVerCompareResult::EQUAL) {
        return true;
    }
    return AgentCardUtils::ShouldKeepStoredCard(incomingCard, storedEntry.card);
}

std::vector<ExtensionAbilityInfo> CollectExtensionInfos(const BundleInfo &bundleInfo)
{
    std::vector<ExtensionAbilityInfo> extensionInfos;
    for (const auto &hapModuleInfo : bundleInfo.hapModuleInfos) {
        extensionInfos.insert(extensionInfos.end(), hapModuleInfo.extensionInfos.begin(),
            hapModuleInfo.extensionInfos.end());
    }
    return extensionInfos;
}
} // namespace

AgentCardMgr &AgentCardMgr::GetInstance()
{
    static AgentCardMgr instance;
    return instance;
}

AgentCardMgr::AgentCardMgr() {}

AgentCardMgr::~AgentCardMgr() {}

int32_t AgentCardMgr::HandleBundleInstall(const std::string &bundleName, int32_t userId)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid bundleName");
        return -1;
    }
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bundleMgrHelper is null");
        return -1;
    }
    BundleInfo bundleInfo;
    ErrCode result = bundleMgrHelper->GetBundleInfoV9(bundleName, GET_AGENT_EXTENSION_BUNDLE_INFO_FLAGS,
        bundleInfo, userId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Get Bundle Info fail");
        return -1;
    }
    std::unordered_map<std::string, AgentCard> incomingCardMap;
    for (auto const &extensionInfo : CollectExtensionInfos(bundleInfo)) {
        if (static_cast<int32_t>(incomingCardMap.size()) >= MAX_AGENT_CARD_SIZE) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "incomingCardMap reached max size %{public}d", MAX_AGENT_CARD_SIZE);
            break;
        }
        if (extensionInfo.type != ExtensionAbilityType::AGENT) {
            continue;
        }

        std::vector<std::string> hapDeviceTypes =
            AgentCardUtils::FindHapDeviceTypes(bundleInfo, extensionInfo.moduleName);

        std::vector<std::string> profileInfos{};
        bundleMgrClient_.GetResConfigFile(extensionInfo, AGENT_CONFIG, profileInfos);
        for (const std::string &profileInfo : profileInfos) {
            if (static_cast<int32_t>(incomingCardMap.size()) >= MAX_AGENT_CARD_SIZE) {
                break;
            }
            if (!json::accept(profileInfo, true)) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "profileInfo is not json format");
                return -1;
            }
            json j = json::parse(profileInfo, nullptr, false, true);
            if (!j.contains("agentCards")) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "profileInfo is not contains agentCards");
                return -1;
            }
            for (auto cardStr : j["agentCards"]) {
                if (static_cast<int32_t>(incomingCardMap.size()) >= MAX_AGENT_CARD_SIZE) {
                    break;
                }
                AgentCard card;
                if (!AgentCard::FromJson(cardStr, card)) {
                    TAG_LOGE(AAFwkTag::SER_ROUTER, "FromJson failed");
                    continue;
                }
                if (card.appInfo == nullptr) {
                    card.appInfo = std::make_shared<AgentAppInfo>();
                }
                card.appInfo->bundleName = bundleName;
                card.appInfo->moduleName = extensionInfo.moduleName;
                card.appInfo->abilityName = extensionInfo.name;
                if (card.type == AgentCardType::LOW_CODE && !bundleInfo.applicationInfo.isSystemApp) {
                    TAG_LOGW(AAFwkTag::SER_ROUTER,
                        "skip low-code card %{public}s because target bundle %{public}s is not system app",
                        card.agentId.c_str(), bundleName.c_str());
                    continue;
                }
                AgentCardUtils::ApplyDeviceTypes(hapDeviceTypes, card);
                incomingCardMap[card.agentId] = card;
            }
        }
    }

    std::vector<StoredAgentCardEntry> storedEntries;
    int32_t ret = AgentCardDbMgr::GetInstance().QueryData(bundleName, userId, storedEntries);
    if (ret != ERR_OK && ret != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "query stored cards failed: %{public}d", ret);
        return ret;
    }

    std::unordered_map<std::string, size_t> storedIndexMap;
    for (size_t i = 0; i < storedEntries.size(); ++i) {
        storedIndexMap.emplace(storedEntries[i].card.agentId, i);
    }

    std::vector<StoredAgentCardEntry> finalEntries = storedEntries;
    for (const auto &entry : incomingCardMap) {
        auto storedIt = storedIndexMap.find(entry.first);
        if (storedIt == storedIndexMap.end()) {
            finalEntries.push_back({entry.second, AgentCardUpdateSource::BUNDLE});
            continue;
        }
        auto &storedEntry = finalEntries[storedIt->second];
        if (ShouldKeepStoredBundleEntry(entry.second, storedEntry)) {
            storedEntry.card.type = entry.second.type;
            continue;
        }
        storedEntry.card = entry.second;
        storedEntry.source = AgentCardUpdateSource::BUNDLE;
    }
    return AgentCardDbMgr::GetInstance().InsertData(bundleName, userId, finalEntries);
}

int32_t AgentCardMgr::HandleBundleUpdate(const std::string &bundleName, int32_t userId)
{
    return HandleBundleInstall(bundleName, userId);
}

int32_t AgentCardMgr::HandleBundleRemove(const std::string &bundleName, int32_t userId)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid bundleName");
        return -1;
    }
    return AgentCardDbMgr::GetInstance().DeleteData(bundleName, userId);
}

int32_t AgentCardMgr::GetAllAgentCards(AgentCardsRawData &cards)
{
    std::vector<StoredAgentCardEntry> entries;
    int32_t resultCode = AgentCardDbMgr::GetInstance().QueryAllData(entries);
    AgentCardsRawData::FromAgentCardVec(ExtractCards(entries), cards);
    return resultCode;
}

int32_t AgentCardMgr::GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
{
    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    std::vector<StoredAgentCardEntry> entries;
    int32_t ret = AgentCardDbMgr::GetInstance().QueryData(bundleName, userId, entries);
    if (ret == ERR_OK) {
        cards = ExtractCards(entries);
    }
    return ret;
}

int32_t AgentCardMgr::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card)
{
    std::vector<AgentCard> cards;
    int32_t resultCode = GetAgentCardsByBundleName(bundleName, cards);
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed: %{public}d", resultCode);
        return resultCode;
    }
    bool found = false;
    for (const AgentCard &agentCard : cards) {
        if (agentCard.agentId == agentId) {
            card = agentCard;
            found = true;
            break;
        }
    }
    if (!found) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not found");
        return ERR_NAME_NOT_FOUND;
    }
    return resultCode;
}

int32_t AgentCardMgr::RegisterAgentCard(const AgentCard &card)
{
    if (!AgentCardUtils::HasRequiredRegisterFields(card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid register params");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (!IsValidSemVer(card.version)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid card version");
        return AAFwk::ERR_INVALID_AGENT_CARD_VERSION;
    }

    AgentCard registerCard = card;
    if (AgentCardUtils::ShouldValidateAppInfo(registerCard) &&
        (registerCard.appInfo->bundleName.empty() || registerCard.appInfo->abilityName.empty())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid card app info");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    if (AgentCardUtils::ShouldValidateBundleAbility(registerCard, userId)) {
        auto validationRet = AgentCardUtils::ValidateBundleAbility(registerCard.appInfo->bundleName,
            registerCard.appInfo->abilityName, userId);
        if (validationRet != ERR_OK) {
            return validationRet;
        }
        validationRet = AgentCardUtils::ValidateSystemAppRequirement(registerCard, userId);
        if (validationRet != ERR_OK) {
            return validationRet;
        }
    }

    std::vector<StoredAgentCardEntry> entries;
    int32_t ret = AgentCardDbMgr::GetInstance().QueryData(registerCard.appInfo->bundleName, userId, entries);
    if (ret != ERR_OK && ret != ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "query data failed: %{public}d", ret);
        return ret;
    }

    auto it = std::find_if(entries.begin(), entries.end(), [&registerCard](const StoredAgentCardEntry &item) {
        return item.card.agentId == registerCard.agentId;
    });
    if (it != entries.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent card already registered");
        return AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER;
    }

    entries.push_back({registerCard, AgentCardUpdateSource::API});
    return AgentCardDbMgr::GetInstance().InsertData(registerCard.appInfo->bundleName, userId, entries);
}

int32_t AgentCardMgr::UpdateAgentCard(const AgentCard &card)
{
    if (card.agentId.empty() || card.appInfo == nullptr || card.appInfo->bundleName.empty() ||
        card.appInfo->abilityName.empty() || !AgentCardUtils::HasValidIconUrl(card.iconUrl)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid update params");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (!IsValidSemVer(card.version)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid card version");
        return AAFwk::ERR_INVALID_AGENT_CARD_VERSION;
    }

    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    int32_t validationRet = ERR_OK;
    if (AgentCardUtils::ShouldValidateBundleAbility(card, userId)) {
        validationRet = AgentCardUtils::ValidateBundleAbility(card.appInfo->bundleName,
            card.appInfo->abilityName, userId);
        if (validationRet != ERR_OK) {
            return validationRet;
        }
    }

    std::vector<StoredAgentCardEntry> entries;
    int32_t ret = AgentCardDbMgr::GetInstance().QueryData(card.appInfo->bundleName, userId, entries);
    if (ret == ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle cards not found");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "query data failed: %{public}d", ret);
        return ret;
    }

    auto it = std::find_if(entries.begin(), entries.end(), [&card](const StoredAgentCardEntry &item) {
        return item.card.agentId == card.agentId;
    });
    if (it == entries.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent card not found");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    if (!AgentCardUtils::IsCardOwnedByAbility(it->card, card.appInfo->bundleName, card.appInfo->abilityName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "stored card owner invalid");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    validationRet = AgentCardUtils::ValidateSystemAppRequirement(card, userId);
    if (validationRet != ERR_OK) {
        return validationRet;
    }
    if (!IsValidSemVer(it->card.version)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "stored card version invalid, allow overwrite");
    } else {
        auto compareResult = CompareSemVer(card.version, it->card.version);
        if (compareResult == SemVerCompareResult::INVALID) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid semver compare");
            return AAFwk::ERR_INVALID_AGENT_CARD_VERSION;
        }
        if (compareResult == SemVerCompareResult::LESS) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "card version too old");
            return AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD;
        }
    }

    it->card = card;
    it->source = AgentCardUpdateSource::API;
    return AgentCardDbMgr::GetInstance().InsertData(card.appInfo->bundleName, userId, entries);
}

int32_t AgentCardMgr::DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
{
    if (bundleName.empty() || agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid delete params");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    std::vector<StoredAgentCardEntry> bundleCards;
    int32_t ret = AgentCardDbMgr::GetInstance().QueryData(bundleName, userId, bundleCards);
    if (ret == ERR_NAME_NOT_FOUND) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle cards not found");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "query data failed: %{public}d", ret);
        return ret;
    }

    auto bundleIt = std::remove_if(bundleCards.begin(), bundleCards.end(),
        [&agentId](const StoredAgentCardEntry &item) { return item.card.agentId == agentId; });
    if (bundleIt == bundleCards.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent card not found in bundle");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    bundleCards.erase(bundleIt, bundleCards.end());

    if (bundleCards.empty()) {
        return AgentCardDbMgr::GetInstance().DeleteData(bundleName, userId);
    }
    return AgentCardDbMgr::GetInstance().InsertData(bundleName, userId, bundleCards);
}
} // namespace AgentRuntime
} // namespace OHOS
