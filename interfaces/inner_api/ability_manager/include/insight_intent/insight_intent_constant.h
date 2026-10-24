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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CONSTANT_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CONSTANT_H

#include "ability_manager_errors.h"

namespace OHOS::AbilityRuntime {
    constexpr char INSIGHT_INTENTS_TYPE_LINK[] = "@InsightIntentLink";
    constexpr char INSIGHT_INTENTS_TYPE_PAGE[] = "@InsightIntentPage";
    constexpr char INSIGHT_INTENTS_TYPE_ENTRY[] = "@InsightIntentEntry";
    constexpr char INSIGHT_INTENTS_TYPE_FUNCTION[] = "@InsightIntentFunctionMethod";
    constexpr char INSIGHT_INTENTS_TYPE_FORM[] = "@InsightIntentForm";

    constexpr char INSIGHT_INTENTS_URI[] = "uri";
    constexpr char INSIGHT_INTENT_UI_ABILITY[] = "uiAbility";
    constexpr char INSIGHT_INTENT_PAGE_PATH[] = "pagePath";
    constexpr char INSIGHT_INTENT_NAVIGATION_ID[] = "navigationId";
    constexpr char INSIGHT_INTENT_NAV_DESTINATION_NAME[] = "navDestinationName";
    constexpr char INSIGHT_INTENT_ABILITY_NAME[] = "abilityName";
    constexpr char INSIGHT_INTENT_FORM_NAME[] = "formName";
    constexpr char INSIGHT_INTENT_EXECUTE_MODE[] = "executeMode";
    constexpr char INSIGHT_INTENT_BUNDLE_NAME[] = "bundleName";
    constexpr char INSIGHT_INTENT_MODULE_NAME[] = "moduleName";
    constexpr char INSIGHT_INTENT_INTENT_NAME[] = "intentName";
    constexpr char INSIGHT_INTENT_SRCENTRY[] = "srcEntry";
    constexpr char INSIGHT_INTENT_DOMAIN[] = "domain";
    constexpr char INSIGHT_INTENT_INTENT_VERSION[] = "intentVersion";
    constexpr char INSIGHT_INTENT_DISPLAY_NAME[] = "displayName";
    constexpr char INSIGHT_INTENT_DISPLAY_DESCRIPTION[] = "displayDescription";
    constexpr char INSIGHT_INTENT_SCHEMA[] = "schema";
    constexpr char INSIGHT_INTENT_ICON[] = "icon";
    constexpr char INSIGHT_INTENT_LLM_DESCRIPTION[] = "llmDescription";
    constexpr char INSIGHT_INTENT_INTENT_TYPE[] = "intentType";
    constexpr char INSIGHT_INTENT_PARAMETERS[] = "parameters";
    constexpr char INSIGHT_INTENT_RESULT[] = "result";
    constexpr char INSIGHT_INTENT_KEYWORDS[] = "keywords";
    constexpr char INSIGHT_INTENT_PERMISSIONS[] = "permissions";
    constexpr char INSIGHT_INTENT_LINK_INFO[] = "linkInfo";
    constexpr char INSIGHT_INTENT_PAGE_INFO[] = "pageInfo";
    constexpr char INSIGHT_INTENT_ENTRY_INFO[] = "entryInfo";
    constexpr char INSIGHT_INTENT_ENTITY_INFO[] = "entities";
    constexpr char INSIGHT_INTENT_FUNCTION_INFO[] = "functionInfo";
    constexpr char INSIGHT_INTENT_FORM_INFO[] = "formInfo";
    constexpr char INSIGHT_INTENT_ENTITY_CLASS_NAME[] = "className";
    constexpr char INSIGHT_INTENT_ENTITY_CATEGORY[] = "entityCategory";
    constexpr char INSIGHT_INTENT_ENTITY_ID[] = "entityId";
    constexpr char INSIGHT_INTENT_ENTITY_PARENT_CLASS_NAME[] = "parentClassName";
    constexpr char INSIGHT_INTENT_ENTITY_PARAMETERS[] = "parameters";
    constexpr char INSIGHT_INTENT_ISCONFIG[] = "isConfig";
    constexpr char INSIGHT_INTENT_UI_EXTENSION[] = "uiExtension";
    constexpr char INSIGHT_INTENT_SERVICE_EXTENSION[] = "serviceExtension";
    constexpr char INSIGHT_INTENT_FORM[] = "form";
    constexpr char INSIGHT_INTENT_ABILITY[] = "ability";
    constexpr char INSIGHT_INTENT_INPUT_PARAMS[] = "inputParams";
    constexpr char INSIGHT_INTENT_OUTPUT_PARAMS[] = "outputParams";
    constexpr char INSIGHT_INTENT_ENTITES[] = "cfgEntities";
    constexpr char INSIGHT_INTENT_DEVELOP_TYPE[] = "developType ";
    constexpr char INSIGHT_INTENT_QUERY_ENTITY_FUNC[] = "onQueryEntity";
    constexpr char INSIGHT_INTENT_ENTITY_SUPPORTED_QUERY_PROPERTIES[] = "supportedQueryProperties";
    enum class InsightIntentExecuteMode {
        UIABILITY_FOREGROUND,
        UIABILITY_BACKGROUND,
        UIEXTENSION_ABILITY,
        SERVICE_EXTENSION_ABILITY,
        // Use default enum value and keep `Count` as the last element
        Count
    };

    enum InsightIntentInnerErr {
        INSIGHT_INTENT_ERR_OK,
        INSIGHT_INTENT_INTERNAL_ERROR,
        INSIGHT_INTENT_EXECUTE_REPLY_FAILED = AAFwk::ERR_INSIGHT_INTENT_EXECUTE_REPLY_FAILED,
    };

    enum GetInsightIntentFlag {
        GET_FULL_INSIGHT_INTENT = 1,
        GET_SUMMARY_INSIGHT_INTENT,
        GET_ENTITY_INFO = 4,
        GET_FULL_INSIGHT_INTENT_ENTITY = 5,
        GET_SUMMARY_INSIGHT_INTENT_ENTITY = 6,
    };

    enum class InsightIntentType : uint8_t {
        DECOR_NONE = 0,
        DECOR_LINK = 1,
        DECOR_PAGE = 2,
        DECOR_FUNC = 3,
        DECOR_FORM = 4,
        DECOR_ENTRY = 5,
        DECOR_QUERY_ENTITY = 6,
    };

    enum InsightIntentReturnMode {
        CALLBACK = 0,
        FUNCTION = 1,
    };
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CONSTANT_H
