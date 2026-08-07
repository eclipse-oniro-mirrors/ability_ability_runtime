/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_insight_intent_driver_utils.h"

#include <cstdint>
#include <memory>

#include "ability_state.h"
#include "insight_intent_execute_result.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsExecuteResult(napi_env env, const AppExecFwk::InsightIntentExecuteResult &result)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    if (result.isQueryEntity) {
        return CreateQueryEntityResult(env, result.queryResults);
    }

    if (result.isDecorator) {
        return result.result ? OHOS::AppExecFwk::CreateJsWantParams(env, *result.result) : objValue;
    }

    napi_set_named_property(env, objValue, "code", CreateJsValue(env, result.code));
    if (result.result != nullptr) {
        napi_set_named_property(env, objValue, "result",
            OHOS::AppExecFwk::CreateJsWantParams(env, *result.result));
    }
    if (result.uris.size() > 0) {
        napi_set_named_property(env, objValue, "uris", CreateNativeArray(env, result.uris));
    }
    napi_set_named_property(env, objValue, "flags", CreateJsValue(env, result.flags));
    if (result.interactionInfo != nullptr && result.interactionInfo->interactionUI != nullptr) {
        napi_value infoObj = nullptr;
        napi_create_object(env, &infoObj);
        napi_value uiObj = nullptr;
        napi_create_object(env, &uiObj);
        napi_set_named_property(env, uiObj, "interactionUIType",
            CreateJsValue(env, result.interactionInfo->interactionUI->interactionUIType));
        if (result.interactionInfo->interactionUI->interactionUIType ==
            AppExecFwk::INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
            auto modalUI = std::static_pointer_cast<AppExecFwk::InteractionModalUIExtension>(
                result.interactionInfo->interactionUI);
            napi_set_named_property(env, uiObj, "bundleName", CreateJsValue(env, modalUI->bundleName));
            napi_set_named_property(env, uiObj, "abilityName", CreateJsValue(env, modalUI->abilityName));
            napi_set_named_property(env, uiObj, "moduleName", CreateJsValue(env, modalUI->moduleName));
            napi_set_named_property(env, uiObj, "uiExtensionType", CreateJsValue(env, modalUI->uiExtensionType));
            napi_set_named_property(env, uiObj, "uri", CreateJsValue(env, modalUI->uri));
            if (modalUI->parameters != nullptr) {
                napi_set_named_property(env, uiObj, "parameters",
                    OHOS::AppExecFwk::CreateJsWantParams(env, *modalUI->parameters));
            }
        }
        napi_set_named_property(env, infoObj, "interactionUI", uiObj);
        napi_set_named_property(env, objValue, "interactionInfo", infoObj);
    }
    return handleEscape.Escape(objValue);
}

napi_value CreateLinkInfoForQuery(napi_env env, const LinkInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "uri", CreateJsValue(env, info.uri));
    return handleEscape.Escape(objValue);
}

napi_value CreatePageInfoForQuery(napi_env env, const PageInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "uiAbility", CreateJsValue(env, info.uiAbility));
    napi_set_named_property(env, objValue, "pagePath", CreateJsValue(env, info.pagePath));
    napi_set_named_property(env, objValue, "navigationId", CreateJsValue(env, info.navigationId));
    napi_set_named_property(env, objValue, "navDestinationName", CreateJsValue(env, info.navDestinationName));
    return handleEscape.Escape(objValue);
}

napi_value CreateEntryInfoForQuery(napi_env env, const EntryInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info.abilityName));
    napi_set_named_property(env, objValue, "executeMode", CreateNativeArray(env, info.executeMode));
    return handleEscape.Escape(objValue);
}

napi_value CreateUiAbilityInfoForQuery(napi_env env, const UIAbilityIntentInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (info.abilityName.empty()) {
        return nullptr;
    }
    
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info.abilityName));
    napi_set_named_property(env, objValue, "executeMode", CreateNativeArray(env, info.supportExecuteMode));
    return handleEscape.Escape(objValue);
}

napi_value CreateUiExtensionInfoForQuery(napi_env env, const UIExtensionIntentInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (info.abilityName.empty()) {
        return nullptr;
    }
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info.abilityName));
    return handleEscape.Escape(objValue);
}

napi_value CreateServiceExtensionInfoForQuery(napi_env env, const ServiceExtensionIntentInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (info.abilityName.empty()) {
        return nullptr;
    }
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info.abilityName));
    return handleEscape.Escape(objValue);
}

napi_value CreateFormIntentInfoForQuery(napi_env env, const FormIntentInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (info.abilityName.empty()) {
        return nullptr;
    }
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info.abilityName));
    napi_set_named_property(env, objValue, "formName", CreateJsValue(env, info.formName));
    return handleEscape.Escape(objValue);
}

napi_value CreateFunctionInfoForQuery(napi_env env, const FunctionInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    return handleEscape.Escape(objValue);
}

napi_value CreateFormInfoForQuery(napi_env env, const FormInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info.abilityName));
    napi_set_named_property(env, objValue, "formName", CreateJsValue(env, info.formName));
    return handleEscape.Escape(objValue);
}

napi_value CreateEntityInfoForArray(napi_env env, const std::vector<EntityInfoForQuery> &infos)
{
    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    napi_status status = napi_create_array_with_length(env, infos.size(), &arrayValue);
    if (status != napi_ok) {
        return nullptr;
    }
    uint32_t index = 0;
    for (const auto &info : infos) {
        napi_value entityInfoObj = CreateJsEntityInfo(env, info);
        NAPI_CALL(env, napi_set_element(env, arrayValue, index++, entityInfoObj));
    }

    return handleEscape.Escape(arrayValue);
}

napi_value CreateJsEntityInfo(napi_env env, const EntityInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objValue));
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null obj");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "className", CreateJsValue(env, info.className));
    napi_set_named_property(env, objValue, "entityId", CreateJsValue(env, info.entityId));
    napi_set_named_property(env, objValue, "entityCategory", CreateJsValue(env, info.entityCategory));
    napi_set_named_property(env, objValue, "parameters", CreateInsightIntentInfoParam(env, info.parameters));
    napi_set_named_property(env, objValue, "parentClassName", CreateJsValue(env, info.parentClassName));
    napi_set_named_property(env, objValue, "isQueryable", CreateJsValue(env, info.isQueryable()));
    if (!info.supportedQueryProperties.empty()) {
        napi_set_named_property(env, objValue, "supportedQueryProperties",
            CreateNativeArray(env, info.supportedQueryProperties));
    }
    return handleEscape.Escape(objValue);
}

napi_value CreateInsightIntentInfoWithJson(napi_env env, const nlohmann::json &jsonObject)
{
    HandleEscape handleEscape(env);
    if (jsonObject.is_object()) {
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        for (const auto &it: jsonObject.items()) {
            if (it.value().is_object() || it.value().is_array()) {
                napi_set_named_property(
                    env, objValue, it.key().c_str(), CreateInsightIntentInfoWithJson(env, it.value()));
            } else if (it.value().is_string()) {
                napi_set_named_property(
                    env, objValue, it.key().c_str(), CreateJsValue(env, it.value().get<std::string>()));
            } else if (it.value().is_boolean()) {
                napi_set_named_property(
                    env, objValue, it.key().c_str(), CreateJsValue(env, it.value().get<bool>()));
            } else if (it.value().is_number()) {
                napi_set_named_property(
                    env, objValue, it.key().c_str(), CreateJsValue(env, it.value().get<double>()));
            }
        }
        return handleEscape.Escape(objValue);
    } else if (jsonObject.is_array()) {
        napi_value arrayValue = nullptr;
        napi_create_array_with_length(env, jsonObject.size(), &arrayValue);
        uint32_t index = 0;
        for (const auto &it: jsonObject) {
            if (it.is_object() || it.is_array()) {
                napi_set_element(env, arrayValue, index++, CreateInsightIntentInfoWithJson(env, it));
            } else if (it.is_string()) {
                napi_set_element(env, arrayValue, index++, CreateJsValue(env, it.get<std::string>()));
            } else if (it.is_boolean()) {
                napi_set_element(env, arrayValue, index++, CreateJsValue(env, it.get<bool>()));
            } else if (it.is_number()) {
                napi_set_element(env, arrayValue, index++, CreateJsValue(env, it.get<double>()));
            }
        }
        return handleEscape.Escape(arrayValue);
    }
    TAG_LOGE(AAFwkTag::INTENT, "Creat js param fail");
    return nullptr;
}

napi_value CreateInsightIntentInfoParam(napi_env env, const std::string &paramStr)
{
    HandleEscape handleEscape(env);
    if (paramStr.empty()) {
        TAG_LOGD(AAFwkTag::INTENT, "paramStr empty");
        return nullptr;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(paramStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "Parse param str fail");
        return nullptr;
    }
    return handleEscape.Escape(CreateInsightIntentInfoWithJson(env, jsonObject));
}

napi_value CreateJsConfigPutParams(napi_env env, const std::vector<std::string> &inputParams)
{
    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, inputParams.size(), &arrayValue));
    uint32_t index = 0;
    for (const auto &paramStr : inputParams) {
        if (paramStr.empty()) {
            continue;
        }
        auto json = nlohmann::json::parse(paramStr, nullptr, false);
        if (json.is_discarded()) {
            TAG_LOGE(AAFwkTag::INTENT, "discarded input param");
            continue;
        }
        napi_value jsValue = CreateInsightIntentInfoWithJson(env, json);
        NAPI_CALL(env, napi_set_element(env, arrayValue, index++, jsValue));
    }
    return handleEscape.Escape(arrayValue);
}

napi_value CreateInsightIntentInfoResult(napi_env env, const std::string &resultStr)
{
    HandleEscape handleEscape(env);
    if (resultStr.empty()) {
        TAG_LOGD(AAFwkTag::INTENT, "resultStr empty");
        return nullptr;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(resultStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "Parse result str fail");
        return nullptr;
    }
    return handleEscape.Escape(CreateInsightIntentInfoWithJson(env, jsonObject));
}

napi_value CreateInsightIntentConfigEntities(napi_env env, const std::string &entities)
{
    HandleEscape handleEscape(env);
    if (entities.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "entities empty");
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        return handleEscape.Escape(objValue);
    }
    nlohmann::json jsonObject = nlohmann::json::parse(entities, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "Parse result str fail");
        return nullptr;
    }
    return handleEscape.Escape(CreateInsightIntentInfoWithJson(env, jsonObject));
}

napi_value CreateInsightIntentInfoForQuery(napi_env env, const InsightIntentInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, info.bundleName));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, info.moduleName));
    napi_set_named_property(env, objValue, "intentName", CreateJsValue(env, info.intentName));
    napi_set_named_property(env, objValue, "domain", CreateJsValue(env, info.domain));
    napi_set_named_property(env, objValue, "intentVersion", CreateJsValue(env, info.intentVersion));
    napi_set_named_property(env, objValue, "displayName", CreateJsValue(env, info.displayName));
    napi_set_named_property(env, objValue, "displayDescription", CreateJsValue(env, info.displayDescription));
    napi_set_named_property(env, objValue, "schema", CreateJsValue(env, info.schema));
    napi_set_named_property(env, objValue, "icon", CreateJsValue(env, info.icon));
    napi_set_named_property(env, objValue, "llmDescription", CreateJsValue(env, info.llmDescription));
    if (info.isConfig) {
        std::string intentType = INSIGHT_INTENTS_TYPE_ENTRY;
        napi_set_named_property(env, objValue, "intentType", CreateJsValue(env, intentType));
    } else {
        napi_set_named_property(env, objValue, "intentType", CreateJsValue(env, info.intentType));
    }
    napi_set_named_property(env, objValue, "parameters", CreateInsightIntentInfoParam(env, info.parameters));
    napi_set_named_property(env, objValue, "result", CreateInsightIntentInfoResult(env, info.result));
    napi_set_named_property(env, objValue, "keywords", CreateNativeArray(env, info.keywords));
    if (!info.develoType.empty()) {
        napi_set_named_property(env, objValue, "developType", CreateJsValue(env, info.develoType));
    }
    napi_set_named_property(env, objValue, "entities", CreateEntityInfoForArray(env, info.entities));
    if (info.intentType == INSIGHT_INTENTS_TYPE_LINK) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateLinkInfoForQuery(env, info.linkInfo));
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_PAGE) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreatePageInfoForQuery(env, info.pageInfo));
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_ENTRY) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateEntryInfoForQuery(env, info.entryInfo));
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_FUNCTION) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateFunctionInfoForQuery(env, info.functionInfo));
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_FORM) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateFormInfoForQuery(env, info.formInfo));
    } else if (info.isConfig) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateEntryInfoForQuery(env, info.entryInfo));
    } else {
        napi_value objVal = nullptr;
        napi_create_object(env, &objValue);
        napi_set_named_property(env, objValue, "subIntentInfo", objVal);
    }
    
    if (info.isConfig) {
        napi_set_named_property(env, objValue, "subIntentInfoForConfiguration", CreateConfigIntentInfo(env, info));
    }
    return handleEscape.Escape(objValue);
}

napi_value CreateConfigIntentInfo(napi_env env, const InsightIntentInfoForQuery &info)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "srcEntry", CreateJsValue(env, info.srcEntry));
    if (info.inputParams.size() > 0) {
        napi_set_named_property(env, objValue, "inputParams", CreateJsConfigPutParams(env, info.inputParams));
    }
    if (info.outputParams.size() > 0) {
        napi_set_named_property(env, objValue, "outputParams", CreateJsConfigPutParams(env, info.outputParams));
    }
    napi_value uiAbilityInfo = CreateUiAbilityInfoForQuery(env, info.uiAbilityIntentInfo);
    if (uiAbilityInfo != nullptr) {
        napi_set_named_property(env, objValue, "uiAbility", uiAbilityInfo);
    }
    napi_value uiExtensionInfo = CreateUiExtensionInfoForQuery(env, info.uiExtensionIntentInfo);
    if (uiExtensionInfo != nullptr) {
        napi_set_named_property(env, objValue, "uiExtension", uiExtensionInfo);
    }
    napi_value serviceExtensionInfo = CreateServiceExtensionInfoForQuery(env, info.serviceExtensionIntentInfo);
    if (serviceExtensionInfo != nullptr) {
        napi_set_named_property(env, objValue, "serviceExtension", serviceExtensionInfo);
    }
    napi_value formInfo = CreateFormIntentInfoForQuery(env, info.formIntentInfo);
    if (formInfo != nullptr) {
        napi_set_named_property(env, objValue, "form", formInfo);
    }
    if (!info.cfgEntities.empty()) {
        napi_set_named_property(env, objValue, "entities", CreateInsightIntentConfigEntities(env, info.cfgEntities));
    }
    return handleEscape.Escape(objValue);
}


napi_value CreateInsightIntentInfoForQueryArray(napi_env env, const std::vector<InsightIntentInfoForQuery> &infos)
{
    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &info : infos) {
        napi_set_element(env, arrayValue, index++, CreateInsightIntentInfoForQuery(env, info));
    }
    return handleEscape.Escape(arrayValue);
}

napi_value CreateQueryEntityResult(napi_env env, const std::vector<std::shared_ptr<AAFwk::WantParams>> &queryResults)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, queryResults.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &item : queryResults) {
        if (item == nullptr) {
            continue;
        }
        napi_set_element(env, arrayValue, index++, OHOS::AppExecFwk::CreateJsWantParams(env, *item));
    }
    return handleEscape.Escape(arrayValue);
}
} // namespace AbilityRuntime
} // namespace OHOS
