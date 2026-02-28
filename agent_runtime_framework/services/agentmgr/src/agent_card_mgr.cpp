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

#include "agent_card.h"
#include "agent_card_db_mgr.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "json_utils.h"

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AppExecFwk;
using json = nlohmann::json;
namespace {
constexpr const char* AGENT_CONFIG = "ohos.extension.agent";
constexpr int32_t BASE_USER_RANGE = 200000;
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
    BundleInfo bundleInfo;
    bool result = bundleMgrClient_.GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO,
        bundleInfo, userId);
    if (!result) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Get Bundle Info fail");
        return -1;
    }
    std::map<std::string, AgentCard> cards;
    for (auto const &extensionInfo : bundleInfo.extensionInfos) {
        if (extensionInfo.type != ExtensionAbilityType::AGENT) {
            continue;
        }

        // Find hapModuleInfo's deviceTypes for this extension once
        std::vector<std::string> hapDeviceTypes;
        for (const auto& hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleName == extensionInfo.moduleName) {
                hapDeviceTypes = hapModuleInfo.deviceTypes;
                break;
            }
        }

        std::vector<std::string> profileInfos{};
        bundleMgrClient_.GetResConfigFile(extensionInfo, AGENT_CONFIG, profileInfos);
        for (const std::string &profileInfo : profileInfos) {
            if (!json::accept(profileInfo)) {
                return -1;
            }
            json j = json::parse(profileInfo);
            if (!j.contains("agentCards")) {
                return -1;
            }
            for (auto cardStr : j["agentCards"]) {
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

                if (!card.appInfo->deviceTypes.empty()) {
                    // Take intersection: only keep deviceTypes that exist in hapModuleInfo
                    std::vector<std::string> filteredDeviceTypes;
                    for (const auto& deviceType : card.appInfo->deviceTypes) {
                        if (std::find(hapDeviceTypes.begin(), hapDeviceTypes.end(),
                            deviceType) != hapDeviceTypes.end()) {
                            filteredDeviceTypes.push_back(deviceType);
                        }
                    }
                    card.appInfo->deviceTypes = filteredDeviceTypes;
                }
                if (card.appInfo->deviceTypes.empty()) {
                    // If deviceTypes is empty, use hapModuleInfo's deviceTypes
                    card.appInfo->deviceTypes = hapDeviceTypes;
                }

                cards[card.agentId] = card;
            }
        }
    }
    std::vector<AgentCard> cardVec;
    for (const auto& [key, value] : cards) {
        cardVec.push_back(value);
    }
    return AgentCardDbMgr::GetInstance().InsertData(bundleName, userId, cardVec);
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
    std::vector<AgentCard> cardVector;
    int32_t resultCode = AgentCardDbMgr::GetInstance().QueryAllData(cardVector);
    AgentCardsRawData::FromAgentCardVec(cardVector, cards);
    return resultCode;
}

int32_t AgentCardMgr::GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
{
    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    return AgentCardDbMgr::GetInstance().QueryData(bundleName, userId, cards);
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
} // namespace AgentRuntime
} // namespace OHOS
