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
#include "ani_enum_convert.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AgentRuntime;

namespace OHOS {
namespace AgentManagerEts {
namespace {
constexpr const char *CLASSNAME_ARRAY = "std.core.Array";
constexpr const char *AGENT_PROVIDER_IMPL_CLASS_NAME = "application.AgentCard.AgentProviderImpl";
constexpr const char *AGENT_APP_INFO_IMPL_CLASS_NAME = "application.AgentCard.AgentAppInfoImpl";
constexpr const char *AGENT_CAPABILITIES_IMPL_CLASS_NAME = "application.AgentCard.AgentCapabilitiesImpl";
constexpr const char *AGENT_SKILL_IMPL_CLASS_NAME = "application.AgentCard.AgentSkillImpl";
constexpr const char *AGENT_CARD_IMPL_CLASS_NAME = "application.AgentCard.AgentCardImpl";
constexpr size_t MAX_ICON_URL_LENGTH = 512;

bool IsValidAgentCardTypeValue(int32_t type)
{
    return type >= static_cast<int32_t>(AgentCardType::APP) &&
        type <= static_cast<int32_t>(AgentCardType::LOW_CODE);
}

bool ParseAgentSkillArray(ani_env *env, ani_object arrayObj, std::vector<std::shared_ptr<AgentSkill>> &skills)
{
    if (env == nullptr || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill array");
        return false;
    }
    ani_int length = 0;
    ani_status status = env->Object_GetPropertyByName_Int(arrayObj, "length", &length);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get length failed: %{public}d", status);
        return false;
    }

    for (ani_int i = 0; i < length; ++i) {
        ani_ref skillRef = nullptr;
        status = env->Object_CallMethodByName_Ref(arrayObj, "$_get", "i:Y", &skillRef, i);
        if (status != ANI_OK || skillRef == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "get skill failed: %{public}d", status);
            return false;
        }
        auto skill = std::make_shared<AgentSkill>();
        if (!ParseEtsAgentSkill(env, reinterpret_cast<ani_object>(skillRef), *skill)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill %{public}d", static_cast<int32_t>(i));
            return false;
        }
        skills.emplace_back(skill);
    }
    return true;
}
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
        TAG_LOGE(AAFwkTag::SER_ROUTER, "find clss %{public}s failed: %{public}d",
            AGENT_PROVIDER_IMPL_CLASS_NAME, status);
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
    status = env->Object_SetPropertyByName_Ref(object, "streaming",
        CreateBoolean(env, static_cast<ani_boolean>(capabilities.streaming)));
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
    if (appInfo.deviceTypes.size() > 0 && !SetStringArrayProperty(env, object, "deviceTypes", appInfo.deviceTypes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set deviceTypes failed");
        return nullptr;
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
    status = env->Object_SetPropertyByName_Ref(object, "type",
        AppExecFwk::CreateInt(env, static_cast<ani_int>(card.type)));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set type failed: %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "version", GetAniString(env, card.version));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "failed status:%{public}d", status);
        return nullptr;
    }
    if (!card.documentationUrl.empty()) {
        status = env->Object_SetPropertyByName_Ref(object,
            "documentationUrl", GetAniString(env, card.documentationUrl));
    }
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

bool ParseEtsAgentProvider(ani_env *env, ani_object object, AgentProvider &provider)
{
    if (env == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "provider not object");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "organization", provider.organization)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad provider.organization");
        return false;
    }
    if (provider.organization.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty provider.organization");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "url", provider.url)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad provider.url");
        return false;
    }
    if (provider.url.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty provider.url");
        return false;
    }
    return true;
}

bool ParseEtsAgentCapabilities(ani_env *env, ani_object object, AgentCapabilities &capabilities)
{
    if (env == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "capabilities not object");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "streaming") &&
        !AppExecFwk::GetBooleanPropertyObject(env, object, "streaming", capabilities.streaming)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad capabilities.streaming");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "pushNotifications") &&
        !AppExecFwk::GetBooleanPropertyObject(env, object, "pushNotifications", capabilities.pushNotifications)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad capabilities.pushNotifications");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "stateTransitionHistory") &&
        !AppExecFwk::GetBooleanPropertyObject(
            env, object, "stateTransitionHistory", capabilities.stateTransitionHistory)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad capabilities.stateTransitionHistory");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "extendedAgentCard") &&
        !AppExecFwk::GetBooleanPropertyObject(env, object, "extendedAgentCard", capabilities.extendedAgentCard)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad capabilities.extendedAgentCard");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "extension") &&
        !AppExecFwk::GetStringProperty(env, object, "extension", capabilities.extension)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad capabilities.extension");
        return false;
    }
    return true;
}

bool ParseEtsAgentAppInfo(ani_env *env, ani_object object, AgentAppInfo &appInfo)
{
    if (env == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "appInfo not object");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "bundleName", appInfo.bundleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad appInfo.bundleName");
        return false;
    }
    if (appInfo.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty appInfo.bundleName");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "abilityName", appInfo.abilityName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad appInfo.abilityName");
        return false;
    }
    if (appInfo.abilityName.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty appInfo.abilityName");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "moduleName") &&
        !AppExecFwk::GetStringProperty(env, object, "moduleName", appInfo.moduleName)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad appInfo.moduleName");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "deviceTypes") &&
        !AppExecFwk::GetStringArrayProperty(env, object, "deviceTypes", appInfo.deviceTypes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad appInfo.deviceTypes");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "minAppVersion") &&
        !AppExecFwk::GetStringProperty(env, object, "minAppVersion", appInfo.minAppVersion)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad appInfo.minAppVersion");
        return false;
    }
    return true;
}

bool ParseEtsAgentSkill(ani_env *env, ani_object object, AgentSkill &skill)
{
    if (env == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "skill not object");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "id", skill.id)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill.id");
        return false;
    }
    if (skill.id.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty skill.id");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "name", skill.name)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill.name");
        return false;
    }
    if (skill.name.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty skill.name");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "description", skill.description)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill.description");
        return false;
    }
    if (skill.description.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty skill.description");
        return false;
    }
    if (!AppExecFwk::GetStringArrayProperty(env, object, "tags", skill.tags)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill.tags");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "examples") &&
        !AppExecFwk::GetStringArrayProperty(env, object, "examples", skill.examples)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill.examples");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "inputModes") &&
        !AppExecFwk::GetStringArrayProperty(env, object, "inputModes", skill.inputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill.inputModes");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "outputModes") &&
        !AppExecFwk::GetStringArrayProperty(env, object, "outputModes", skill.outputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill.outputModes");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "extension") &&
        !AppExecFwk::GetStringProperty(env, object, "extension", skill.extension)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad skill.extension");
        return false;
    }
    return true;
}

bool ParseEtsAgentCard(ani_env *env, ani_object object, AgentCard &card)
{
    if (env == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "card not object");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "agentId", card.agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.agentId");
        return false;
    }
    if (card.agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty card.agentId");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "name", card.name)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.name");
        return false;
    }
    if (card.name.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty card.name");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "description", card.description)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.description");
        return false;
    }
    if (card.description.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty card.description");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "version", card.version)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.version");
        return false;
    }
    if (card.version.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty card.version");
        return false;
    }
    if (!AppExecFwk::GetStringArrayProperty(env, object, "defaultInputModes", card.defaultInputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.defaultInputModes");
        return false;
    }
    if (!AppExecFwk::GetStringArrayProperty(env, object, "defaultOutputModes", card.defaultOutputModes)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.defaultOutputModes");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "category", card.category)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.category");
        return false;
    }
    if (card.category.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty card.category");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "type")) {
        ani_ref obj = nullptr;
        if (!AppExecFwk::GetRefProperty(env, object, "type", obj)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.type");
            return false;
        }
        if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(
            env, reinterpret_cast<ani_enum_item>(obj), card.type)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "convert failed for card.type");
            return false;
        }
        if (!IsValidAgentCardTypeValue(static_cast<int32_t>(card.type))) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid card.type %{public}d", card.type);
            return false;
        }
    }

    ani_ref ref = nullptr;
    if (!AppExecFwk::GetRefProperty(env, object, "skills", ref)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "missing card.skills");
        return false;
    }
    if (ref == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null card.skills");
        return false;
    }
    if (!ParseAgentSkillArray(env, reinterpret_cast<ani_object>(ref), card.skills)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.skills");
        return false;
    }

    if (AppExecFwk::IsExistsProperty(env, object, "provider")) {
        ref = nullptr;
        if (!AppExecFwk::GetRefProperty(env, object, "provider", ref) || ref == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.provider");
            return false;
        }
        auto provider = std::make_shared<AgentProvider>();
        if (!ParseEtsAgentProvider(env, reinterpret_cast<ani_object>(ref), *provider)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.provider");
            return false;
        }
        card.provider = provider;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "capabilities")) {
        ref = nullptr;
        if (!AppExecFwk::GetRefProperty(env, object, "capabilities", ref) || ref == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.capabilities");
            return false;
        }
        auto capabilities = std::make_shared<AgentCapabilities>();
        if (!ParseEtsAgentCapabilities(env, reinterpret_cast<ani_object>(ref), *capabilities)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.capabilities");
            return false;
        }
        card.capabilities = capabilities;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "documentationUrl") &&
        !AppExecFwk::GetStringProperty(env, object, "documentationUrl", card.documentationUrl)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.documentationUrl");
        return false;
    }
    if (!AppExecFwk::GetStringProperty(env, object, "iconUrl", card.iconUrl)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.iconUrl");
        return false;
    }
    if (card.iconUrl.empty() || card.iconUrl.length() > MAX_ICON_URL_LENGTH) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "invalid card.iconUrl");
        return false;
    }
    if (AppExecFwk::IsExistsProperty(env, object, "extension") &&
        !AppExecFwk::GetStringProperty(env, object, "extension", card.extension)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.extension");
        return false;
    }

    ref = nullptr;
    if (!AppExecFwk::GetRefProperty(env, object, "appInfo", ref)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "missing card.appInfo");
        return false;
    }
    if (ref == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null card.appInfo");
        return false;
    }
    auto appInfo = std::make_shared<AgentAppInfo>();
    if (!ParseEtsAgentAppInfo(env, reinterpret_cast<ani_object>(ref), *appInfo)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "bad card.appInfo");
        return false;
    }
    card.appInfo = appInfo;
    return true;
}
} // namespace AgentManagerEts
} // namespace OHOS
