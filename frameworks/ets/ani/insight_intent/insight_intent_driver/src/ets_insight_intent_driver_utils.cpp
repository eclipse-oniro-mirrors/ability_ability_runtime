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

#include "ets_insight_intent_driver_utils.h"

#include <cstdint>

#include "ability_state.h"
#include "ani_common_json_util.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* CLASSNAME_ARRAY = "std.core.Array";
constexpr const char* EXECUTE_MODE_FOR_CONFIGURATION =
    "@ohos.app.ability.insightIntentDriver.insightIntentDriver.ExecuteModeForConfiguration";
constexpr const char* EXECUTE_MODE = "@ohos.app.ability.insightIntent.insightIntent.ExecuteMode";
constexpr const char* SERVICE_EXTENSION_INTENT_INFO_INNER =
    "@ohos.app.ability.insightIntentDriver.insightIntentDriver.ServiceExtensionIntentInfoInner";
constexpr const char* SUB_INTENT_INFO_FOR_CONFIGURATION_INNER =
    "@ohos.app.ability.insightIntentDriver.insightIntentDriver.SubIntentInfoForConfigurationInner";
constexpr const char* UIEXTENSION_INTENT_INFO_INNER =
    "@ohos.app.ability.insightIntentDriver.insightIntentDriver.UIExtensionIntentInfoInner";
}
ani_object CreateEtsEntityInfoForArray(ani_env *env, const std::vector<EntityInfoForQuery> &infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &info : infos) {
        ani_ref infoRef = CreateEtsEntityInfo(env, info);
        if (infoRef == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null infoRef");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, infoRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsEntityInfo(ani_env *env, const EntityInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.EntityInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    env->Object_SetPropertyByName_Ref(objValue, "className", AppExecFwk::GetAniString(env, info.className));
    env->Object_SetPropertyByName_Ref(objValue, "entityId", AppExecFwk::GetAniString(env, info.entityId));
    env->Object_SetPropertyByName_Ref(objValue, "entityCategory", AppExecFwk::GetAniString(env, info.entityCategory));
    env->Object_SetPropertyByName_Ref(objValue, "parameters", CreateInsightIntentInfoParam(env, info.parameters));
    env->Object_SetPropertyByName_Ref(objValue, "parentClassName", AppExecFwk::GetAniString(env, info.parentClassName));
    env->Object_SetPropertyByName_Ref(objValue, "isQueryable", AppExecFwk::CreateBoolean(env, info.isQueryable()));
    if (!info.supportedQueryProperties.empty()) {
        ani_object stringArray = nullptr;
        AppExecFwk::WrapArrayString(env, stringArray, info.supportedQueryProperties);
        env->Object_SetPropertyByName_Ref(objValue, "supportedQueryProperties", stringArray);
    }
    return objValue;
}

ani_object CreateEtsLinkInfoForQuery(ani_env *env, const LinkInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.LinkIntentInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "uri", AppExecFwk::GetAniString(env, info.uri));
    return objValue;
}

ani_object CreateEtsPageInfoForQuery(ani_env *env, const PageInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.PageIntentInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "uiAbility", AppExecFwk::GetAniString(env, info.uiAbility));
    env->Object_SetPropertyByName_Ref(objValue, "pagePath", AppExecFwk::GetAniString(env, info.pagePath));
    env->Object_SetPropertyByName_Ref(objValue, "navigationId", AppExecFwk::GetAniString(env, info.navigationId));
    env->Object_SetPropertyByName_Ref(
        objValue, "navDestinationName", AppExecFwk::GetAniString(env, info.navDestinationName));
    return objValue;
}

ani_object CreateEtsEntryInfoForQuery(ani_env *env, const EntryInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.EntryIntentInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    env->Object_SetPropertyByName_Ref(
        objValue, "executeMode", CreateExecuteModeArray(env, info.executeMode, EXECUTE_MODE));
    return objValue;
}

ani_object CreateEtsUiAbilityInfoForQuery(ani_env *env, const UIAbilityIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.UIAbilityIntentInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if (info.abilityName.empty()) {
        return nullptr;
    }
    
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    env->Object_SetPropertyByName_Ref(objValue, "executeMode", CreateExecuteModeArray(
        env, info.supportExecuteMode, EXECUTE_MODE_FOR_CONFIGURATION));
    return objValue;
}

ani_object CreateExecuteModeArray(ani_env *env, const std::vector<AppExecFwk::ExecuteMode> &executeModes,
    const std::string &executeModeName)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, executeModes.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_size index = 0;
    for (auto &executeMode : executeModes) {
        ani_enum_item modeItem = nullptr;
        OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
            env, executeModeName.c_str(), executeMode, modeItem);
        if (modeItem == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null infoRef");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, modeItem);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsUiExtensionInfoForQuery(ani_env *env, const UIExtensionIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass(UIEXTENSION_INTENT_INFO_INNER, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if (info.abilityName.empty()) {
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    return objValue;
}

ani_object CreateEtsServiceExtensionInfoForQuery(ani_env *env, const ServiceExtensionIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass(SERVICE_EXTENSION_INTENT_INFO_INNER, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if (info.abilityName.empty()) {
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    return objValue;
}

ani_object CreateEtsFormIntentInfoForQuery(ani_env *env, const FormIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.FormIntentInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if (info.abilityName.empty()) {
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    env->Object_SetPropertyByName_Ref(objValue, "formName", AppExecFwk::GetAniString(env, info.formName));
    return objValue;
}

ani_object CreateEtsFunctionInfoForQuery(ani_env *env, const FunctionInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.FunctionIntentInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    return objValue;
}

ani_object CreateEtsFormInfoForQuery(ani_env *env, const FormInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.FormIntentInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    env->Object_SetPropertyByName_Ref(objValue, "formName", AppExecFwk::GetAniString(env, info.formName));
    return objValue;
}

ani_object CreateInsightIntentInfoParam(ani_env *env, const std::string &paramStr)
{
    ani_object recordObject;
    if (paramStr.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "paramStr empty");
        AppExecFwk::CreateEmptyAniRecord(env, recordObject);
        return recordObject;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(paramStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "Parse param str fail");
        AppExecFwk::CreateEmptyAniRecord(env, recordObject);
        return recordObject;
    }

    if (!AppExecFwk::CreateAniRecordFromJson(env, jsonObject, recordObject)) {
        TAG_LOGE(AAFwkTag::INTENT, "failed to create record object from json");
        return nullptr;
    }
    return recordObject;
}

ani_object CreateInsightIntentInfoWithJson(ani_env *env, const nlohmann::json &jsonObject)
{
    ani_object recordObject = nullptr;
    if (!AppExecFwk::CreateAniRecordFromJson(env, jsonObject, recordObject)) {
        TAG_LOGE(AAFwkTag::INTENT, "failed to create InsightIntent record object from JSON");
        return nullptr;
    }
    return recordObject;
}

ani_object CreateEtsInsightIntentInfoForQueryArray(ani_env *env, const std::vector<InsightIntentInfoForQuery> &infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &intentInfo : infos) {
        ani_ref intentInfoRef = CreateEtsInsightIntentInfoForQuery(env, intentInfo);
        if (intentInfoRef == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null intentInfoRef");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, intentInfoRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsConfigPutParams(ani_env *env, const std::vector<std::string> &putParams)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, putParams.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &putParam : putParams) {
        ani_ref putParamRef =  CreateInsightIntentInfoParam(env, putParam);
        if (putParamRef == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null putParamRef");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, putParamRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsConfigIntentInfo(ani_env *env, const InsightIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass(SUB_INTENT_INFO_FOR_CONFIGURATION_INNER, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "srcEntry", AppExecFwk::GetAniString(env, info.srcEntry));
    if (info.inputParams.size() > 0) {
        env->Object_SetPropertyByName_Ref(objValue, "inputParams", CreateEtsConfigPutParams(env, info.inputParams));
    }
    if (info.outputParams.size() > 0) {
        env->Object_SetPropertyByName_Ref(objValue, "outputParams", CreateEtsConfigPutParams(env, info.outputParams));
    }
    ani_object uiAbilityInfo = CreateEtsUiAbilityInfoForQuery(env, info.uiAbilityIntentInfo);
    if (uiAbilityInfo != nullptr) {
        env->Object_SetPropertyByName_Ref(objValue, "uiAbility", uiAbilityInfo);
    }
    ani_object uiExtensionInfo = CreateEtsUiExtensionInfoForQuery(env, info.uiExtensionIntentInfo);
    if (uiExtensionInfo != nullptr) {
        env->Object_SetPropertyByName_Ref(objValue, "uiExtension", uiExtensionInfo);
    }
    ani_object serviceExtensionInfo = CreateEtsServiceExtensionInfoForQuery(env, info.serviceExtensionIntentInfo);
    if (serviceExtensionInfo != nullptr) {
        env->Object_SetPropertyByName_Ref(objValue, "serviceExtension", serviceExtensionInfo);
    }
    ani_object formIntentInfo = CreateEtsFormIntentInfoForQuery(env, info.formIntentInfo);
    if (formIntentInfo != nullptr) {
        env->Object_SetPropertyByName_Ref(objValue, "form", formIntentInfo);
    }
    if (!info.cfgEntities.empty()) {
        env->Object_SetPropertyByName_Ref(objValue, "entities", CreateInsightIntentInfoParam(env, info.cfgEntities));
    }
    
    return objValue;
}

ani_object CreateEtsInsightIntentInfoForQuery(ani_env *env, const InsightIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntentDriver.insightIntentDriver.InsightIntentInfoInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if (info.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "info is empty");
        return objValue;
    }

    SetInsightIntentInfo(env, objValue, info);
    return objValue;
}

void SetInsightIntentInfo(ani_env *env, ani_object objValue, const InsightIntentInfoForQuery &info)
{
    env->Object_SetPropertyByName_Ref(objValue, "bundleName", AppExecFwk::GetAniString(env, info.bundleName));
    env->Object_SetPropertyByName_Ref(objValue, "moduleName", AppExecFwk::GetAniString(env, info.moduleName));
    env->Object_SetPropertyByName_Ref(objValue, "intentName", AppExecFwk::GetAniString(env, info.intentName));
    env->Object_SetPropertyByName_Ref(objValue, "domain", AppExecFwk::GetAniString(env, info.domain));
    env->Object_SetPropertyByName_Ref(objValue, "intentVersion", AppExecFwk::GetAniString(env, info.intentVersion));
    env->Object_SetPropertyByName_Ref(objValue, "displayName", AppExecFwk::GetAniString(env, info.displayName));
    env->Object_SetPropertyByName_Ref(
        objValue, "displayDescription", AppExecFwk::GetAniString(env, info.displayDescription));
    env->Object_SetPropertyByName_Ref(objValue, "schema", AppExecFwk::GetAniString(env, info.schema));
    env->Object_SetPropertyByName_Ref(objValue, "icon", AppExecFwk::GetAniString(env, info.icon));
    env->Object_SetPropertyByName_Ref(objValue, "llmDescription", AppExecFwk::GetAniString(env, info.llmDescription));
    if (info.isConfig) {
        std::string intentType = INSIGHT_INTENTS_TYPE_ENTRY;
        env->Object_SetPropertyByName_Ref(objValue, "intentType", AppExecFwk::GetAniString(env, intentType));
    } else {
        env->Object_SetPropertyByName_Ref(objValue, "intentType", AppExecFwk::GetAniString(env, info.intentType));
    }

    env->Object_SetPropertyByName_Ref(objValue, "parameters", CreateInsightIntentInfoParam(env, info.parameters));
    env->Object_SetPropertyByName_Ref(objValue, "result", CreateInsightIntentInfoParam(env, info.result));
    ani_object stringArray = nullptr;
    AppExecFwk::WrapArrayString(env, stringArray, info.keywords);
    env->Object_SetPropertyByName_Ref(objValue, "keywords", stringArray);
    if (!info.develoType.empty()) {
        env->Object_SetPropertyByName_Ref(objValue, "developType", AppExecFwk::GetAniString(env, info.develoType));
    }
    env->Object_SetPropertyByName_Ref(objValue, "entities", CreateEtsEntityInfoForArray(env, info.entities));
    if (info.intentType == INSIGHT_INTENTS_TYPE_LINK) {
        env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsLinkInfoForQuery(env, info.linkInfo));
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_PAGE) {
        env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsPageInfoForQuery(env, info.pageInfo));
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_ENTRY) {
        env->Object_SetPropertyByName_Ref(
            objValue, "subIntentInfo", CreateEtsEntryInfoForQuery(env, info.entryInfo));
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_FUNCTION) {
        env->Object_SetPropertyByName_Ref(
            objValue, "subIntentInfo", CreateEtsFunctionInfoForQuery(env, info.functionInfo));
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_FORM) {
        env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsFormInfoForQuery(env, info.formInfo));
    } else {
        if (info.isConfig) {
            env->Object_SetPropertyByName_Ref(
                objValue, "subIntentInfo", CreateEtsEntryInfoForQuery(env, info.entryInfo));
        }
    }
    if (info.isConfig) {
        env->Object_SetPropertyByName_Ref(
            objValue, "subIntentInfoForConfiguration", CreateEtsConfigIntentInfo(env, info));
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
