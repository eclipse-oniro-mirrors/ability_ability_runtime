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

#include "js_agent_manager_utils.h"

#include <cstdint>

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AgentRuntime {
napi_value CreateJsAgentProvider(napi_env env, const AgentProvider &provider)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create provider");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "organization", CreateJsValue(env, provider.organization));
    napi_set_named_property(env, object, "url", CreateJsValue(env, provider.url));
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentAppInfo(napi_env env, const AgentAppInfo &appInfo)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create appInfo");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    if (!appInfo.bundleName.empty()) {
        napi_set_named_property(env, object, "bundleName", CreateJsValue(env, appInfo.bundleName));
    }
    if (!appInfo.moduleName.empty()) {
        napi_set_named_property(env, object, "moduleName", CreateJsValue(env, appInfo.moduleName));
    }
    if (!appInfo.abilityName.empty()) {
        napi_set_named_property(env, object, "abilityName", CreateJsValue(env, appInfo.abilityName));
    }
    if (!appInfo.deviceTypes.empty()) {
        napi_set_named_property(env, object, "deviceTypes", CreateNativeArray(env, appInfo.deviceTypes));
    }
    if (!appInfo.minAppVersion.empty()) {
        napi_set_named_property(env, object, "minAppVersion", CreateJsValue(env, appInfo.minAppVersion));
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentCapabilities(napi_env env, const AgentCapabilities &capabilities)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create capabilities");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "streaming", CreateJsValue(env, capabilities.streaming));
    napi_set_named_property(env, object, "pushNotifications", CreateJsValue(env, capabilities.pushNotifications));
    napi_set_named_property(env, object, "stateTransitionHistory",
        CreateJsValue(env, capabilities.stateTransitionHistory));
    if (!capabilities.extension.empty()) {
        napi_set_named_property(env, object, "extension", CreateJsValue(env, capabilities.extension));
    }
    napi_set_named_property(env, object, "extendedAgentCard", CreateJsValue(env, capabilities.extendedAgentCard));
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentSkill(napi_env env, const AgentSkill &skill)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create skill");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "id", CreateJsValue(env, skill.id));
    napi_set_named_property(env, object, "name", CreateJsValue(env, skill.name));
    napi_set_named_property(env, object, "description", CreateJsValue(env, skill.description));
    napi_set_named_property(env, object, "tags", CreateNativeArray(env, skill.tags));
    napi_set_named_property(env, object, "examples", CreateNativeArray(env, skill.examples));
    napi_set_named_property(env, object, "inputModes", CreateNativeArray(env, skill.inputModes));
    napi_set_named_property(env, object, "outputModes", CreateNativeArray(env, skill.outputModes));
    napi_set_named_property(env, object, "extension", CreateJsValue(env, skill.extension));
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentSkillArray(napi_env env, const std::vector<std::shared_ptr<AgentSkill>> &skills)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create skill array");
    napi_value object = nullptr;
    napi_status status = napi_create_array_with_length(env, skills.size(), &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }

    uint32_t index = 0;
    for (auto skill : skills) {
        if (skill) {
            napi_set_element(env, object, index++, CreateJsAgentSkill(env, *skill));
        }
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentCard(napi_env env, const AgentCard &card)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create card");
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "agentId", CreateJsValue(env, card.agentId));
    napi_set_named_property(env, object, "name", CreateJsValue(env, card.name));
    napi_set_named_property(env, object, "description", CreateJsValue(env, card.description));
    napi_set_named_property(env, object, "version", CreateJsValue(env, card.version));
    napi_set_named_property(env, object, "documentationUrl", CreateJsValue(env, card.documentationUrl));
    napi_set_named_property(env, object, "defaultInputModes", CreateNativeArray(env, card.defaultInputModes));
    napi_set_named_property(env, object, "defaultOutputModes", CreateNativeArray(env, card.defaultOutputModes));
    if (card.provider) {
        napi_set_named_property(env, object, "provider", CreateJsAgentProvider(env, *(card.provider)));
    }
    if (card.capabilities) {
        napi_set_named_property(env, object, "capabilities", CreateJsAgentCapabilities(env, *(card.capabilities)));
    }
    napi_set_named_property(env, object, "skills", CreateJsAgentSkillArray(env, card.skills));
    if (!card.extension.empty()) {
        napi_set_named_property(env, object, "extension", CreateJsValue(env, card.extension));
    }
    if (!card.category.empty()) {
        napi_set_named_property(env, object, "category", CreateJsValue(env, card.category));
    }
    if (!card.iconUrl.empty()) {
        napi_set_named_property(env, object, "iconUrl", CreateJsValue(env, card.iconUrl));
    }
    if (card.appInfo) {
        napi_set_named_property(env, object, "appInfo", CreateJsAgentAppInfo(env, *(card.appInfo)));
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

napi_value CreateJsAgentCardArray(napi_env env, const std::vector<AgentCard> &cards)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create card array");
    napi_value object = nullptr;
    napi_status status = napi_create_array_with_length(env, cards.size(), &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null obj");
        return nullptr;
    }

    uint32_t index = 0;
    for (const auto &card : cards) {
        napi_set_element(env, object, index++, CreateJsAgentCard(env, card));
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}
}  // namespace AgentRuntime
}  // namespace OHOS
