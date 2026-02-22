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

#include "ets_agent_manager_utils.h"

#include "ani_common_util.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AgentRuntime;

namespace OHOS {
namespace AgentManagerEts {
namespace {
constexpr const char *CLASSNAME_ARRAY = "std.core.Array";
constexpr const char *AGENT_PROVIDER_IMPL_CLASS_NAME = "@ohos.app.agent.AgentCard.AgentProviderImpl";
constexpr const char *AGENT_APP_INFO_IMPL_CLASS_NAME = "@ohos.app.agent.AgentCard.AgentAppInfoImpl";
constexpr const char *AGENT_CAPABILITIES_IMPL_CLASS_NAME = "@ohos.app.agent.AgentCard.AgentCapabilitiesImpl";
constexpr const char *AGENT_SKILL_IMPL_CLASS_NAME = "@ohos.app.agent.AgentCard.AgentSkillImpl";
constexpr const char *AGENT_CARD_IMPL_CLASS_NAME = "@ohos.app.agent.AgentCard.AgentCardImpl";
}  // namespace

ani_object CreateEtsAgentProvider(ani_env *env, const AgentProvider &provider)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(AGENT_PROVIDER_IMPL_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find clss %{public}s failed: %{public}d", AGENT_PROVIDER_IMPL_CLASS_NAME, status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find ctor method failed: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "new object failed: %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(
        object, "organization", GetAniString(env, provider.organization));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "url", GetAniString(env, provider.url));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    return object;
}

ani_object CreateEtsAgentCapabilities(ani_env *env, const AgentCapabilities &capabilities)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(AGENT_CAPABILITIES_IMPL_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find clss %{public}s failed: %{public}d",
            AGENT_CAPABILITIES_IMPL_CLASS_NAME, status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find ctor method failed: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "new object failed: %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Boolean(object, "streaming", capabilities.streaming);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "pushNotifications",
        CreateBoolean(env, static_cast<ani_boolean>(capabilities.pushNotifications)));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "stateTransitionHistory",
        CreateBoolean(env, static_cast<ani_boolean>(capabilities.stateTransitionHistory)));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    if (!capabilities.extension.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "extension", GetAniString(env, capabilities.extension));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set extension failed: %{public}d", status);
            return nullptr;
        }
    }
    status = env->Object_SetPropertyByName_Ref(object, "extendedAgentCard",
        CreateBoolean(env, static_cast<ani_boolean>(capabilities.extendedAgentCard)));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set extendedAgentCard failed: %{public}d", status);
        return nullptr;
    }
    return object;
}

ani_object CreateEtsAgentAppInfo(ani_env *env, const AgentAppInfo &appInfo)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(AGENT_APP_INFO_IMPL_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find clss %{public}s failed: %{public}d",
            AGENT_APP_INFO_IMPL_CLASS_NAME, status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find ctor method failed: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "new object failed: %{public}d", status);
        return nullptr;
    }
    if (!appInfo.bundleName.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "bundleName", GetAniString(env, appInfo.bundleName));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set bundleName failed: %{public}d", status);
            return nullptr;
        }
    }
    if (!appInfo.moduleName.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "moduleName", GetAniString(env, appInfo.moduleName));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set moduleName failed: %{public}d", status);
            return nullptr;
        }
    }
    if (!appInfo.abilityName.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "abilityName", GetAniString(env, appInfo.abilityName));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set abilityName failed: %{public}d", status);
            return nullptr;
        }
    }
    if (!appInfo.deviceTypes.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "deviceTypes", GetAniString(env, appInfo.deviceTypes));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set deviceTypes failed: %{public}d", status);
            return nullptr;
        }
    }
    if (!appInfo.minAppVersion.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "minAppVersion", GetAniString(env, appInfo.minAppVersion));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set minAppVersion failed: %{public}d", status);
            return nullptr;
        }
    }
    return object;
}

ani_object CreateEtsAgentSkill(ani_env *env, const AgentSkill &skill)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(AGENT_SKILL_IMPL_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find clss %{public}s failed: %{public}d", AGENT_SKILL_IMPL_CLASS_NAME, status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find ctor method failed: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "new object failed: %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "id", GetAniString(env, skill.id));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "name", GetAniString(env, skill.name));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "description", GetAniString(env, skill.description));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    if (skill.tags.size() > 0 && !SetStringArrayProperty(env, object, "tags", skill.tags)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set tags failed");
        return nullptr;
    }
    if (skill.examples.size() > 0 && !SetStringArrayProperty(env, object, "examples", skill.examples)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set examples failed");
        return nullptr;
    }
    if (skill.inputModes.size() > 0 && !SetStringArrayProperty(env, object, "inputModes", skill.inputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set inputModes failed");
        return nullptr;
    }
    if (skill.outputModes.size() > 0 && !SetStringArrayProperty(env, object, "outputModes", skill.outputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set outputModes failed");
        return nullptr;
    }
    if (!skill.extension.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "extension", GetAniString(env, skill.extension));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set extension failed: %{public}d", status);
            return nullptr;
        }
    }
    return object;
}

ani_object CreateEtsAgentSkillArray(ani_env *env, const std::vector<std::shared_ptr<AgentSkill>> &skills)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }

    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->FindClass(CLASSNAME_ARRAY, &arrayCls)) != ANI_OK || arrayCls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find array class failed: %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    if ((env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor)) != ANI_OK || arrayCtor == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find ctor failed: %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj;
    if ((status = env->Object_New(arrayCls, arrayCtor, &arrayObj, skills.size())) != ANI_OK || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_New array failed: %{public}d", status);
        return arrayObj;
    }
    ani_size index = 0;
    for (auto skill : skills) {
        if (skill == nullptr) {
            continue;
        }
        ani_object aniSkill = CreateEtsAgentSkill(env, *skill);
        if (aniSkill == nullptr) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "null aniSkill");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniSkill);
        if (status != ANI_OK) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "Object_CallMethodByName_Void failed: %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsAgentCard(ani_env *env, const AgentCard &card)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(AGENT_CARD_IMPL_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find clss %{public}s failed: %{public}d", AGENT_CARD_IMPL_CLASS_NAME, status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find ctor method failed: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "new object failed: %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "name", GetAniString(env, card.name));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "description", GetAniString(env, card.description));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "agentId", GetAniString(env, card.agentId));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "version", GetAniString(env, card.version));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "documentationUrl", GetAniString(env, card.documentationUrl));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    if (card.provider) {
        status = env->Object_SetPropertyByName_Ref(object, "provider", CreateEtsAgentProvider(env, *(card.provider)));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set provider failed:%{public}d", status);
            return nullptr;
        }
    }
    if (card.capabilities) {
        status = env->Object_SetPropertyByName_Ref(object, "capabilities",
            CreateEtsAgentCapabilities(env, *(card.capabilities)));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
            return nullptr;
        }
    }
    if (card.defaultInputModes.size() > 0 &&
        !SetStringArrayProperty(env, object, "defaultInputModes", card.defaultInputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set defaultInputModes failed");
        return nullptr;
    }
    if (card.defaultOutputModes.size() > 0 &&
        !SetStringArrayProperty(env, object, "defaultOutputModes", card.defaultOutputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set defaultOutputModes failed");
        return nullptr;
    }
    if (card.skills.size() > 0) {
        status = env->Object_SetPropertyByName_Ref(object, "skills", CreateEtsAgentSkillArray(env, card.skills));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
            return nullptr;
        }
    }
    if (!card.extension.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "extension", GetAniString(env, card.extension));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set extension failed: %{public}d", status);
            return nullptr;
        }
    }
    if (!card.category.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "category", GetAniString(env, card.category));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set category failed: %{public}d", status);
            return nullptr;
        }
    }
    if (!card.iconUrl.empty()) {
        status = env->Object_SetPropertyByName_Ref(object, "iconUrl", GetAniString(env, card.iconUrl));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set iconUrl failed: %{public}d", status);
            return nullptr;
        }
    }
    if (card.appInfo) {
        status = env->Object_SetPropertyByName_Ref(object, "appInfo", CreateEtsAgentAppInfo(env, *(card.appInfo)));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "set appInfo failed: %{public}d", status);
            return nullptr;
        }
    }
    return object;
}

ani_object CreateEtsAgentCardArray(ani_env *env, const std::vector<AgentCard> &cards)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }

    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->FindClass(CLASSNAME_ARRAY, &arrayCls)) != ANI_OK || arrayCls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find array class failed: %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    if ((env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor)) != ANI_OK || arrayCtor == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find ctor failed: %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj;
    if ((status = env->Object_New(arrayCls, arrayCtor, &arrayObj, cards.size())) != ANI_OK || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_New array failed: %{public}d", status);
        return arrayObj;
    }
    ani_size index = 0;
    for (const auto &card : cards) {
        ani_object aniCard = CreateEtsAgentCard(env, card);
        if (aniCard == nullptr) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "null aniCard");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniCard);
        if (status != ANI_OK) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "Object_CallMethodByName_Void failed: %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}
} // namespace AgentManagerEts
} // namespace OHOS
