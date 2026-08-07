/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "napi_common_execute_result.h"

#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "want_params.h"
#include <memory>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
bool UnwrapResultOfExecuteResult(napi_env env, napi_value param, InsightIntentExecuteResult &executeResult)
{
    napi_value result = nullptr;
    napi_get_named_property(env, param, "result", &result);
    if (result != nullptr) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, result, &valueType);
        if (valueType != napi_object) {
            TAG_LOGE(AAFwkTag::JSNAPI, "type not function");
            return false;
        }
        auto wp = std::make_shared<AAFwk::WantParams>();
        if (!AppExecFwk::UnwrapWantParams(env, result, *wp)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrap want failed");
            return false;
        }
        if (!executeResult.CheckResult(wp)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Check wp fail");
            return false;
        }
        executeResult.result = wp;
    }
    return true;
}

bool UnwrapResultOfDecoratorExecuteResult(napi_env env, napi_value param, InsightIntentExecuteResult &executeResult)
{
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "decorator param null");
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, param, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::JSNAPI, "type not object");
        return false;
    }
    auto wp = std::make_shared<AAFwk::WantParams>();
    if (!AppExecFwk::UnwrapWantParams(env, param, *wp)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrap want failed");
        return false;
    }
    if (!executeResult.CheckResult(wp)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Check wp fail");
        return false;
    }
    executeResult.result = wp;
    return true;
}

bool UnwrapModalUIExtensionFields(napi_env env, napi_value obj,
    std::shared_ptr<AppExecFwk::InteractionModalUIExtension> &modalUI)
{
    if (modalUI == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null modalUI");
        return false;
    }
    if (!UnwrapStringByPropertyName(env, obj, "bundleName", modalUI->bundleName) ||
        !UnwrapStringByPropertyName(env, obj, "abilityName", modalUI->abilityName) ||
        !UnwrapStringByPropertyName(env, obj, "moduleName", modalUI->moduleName) ||
        !UnwrapStringByPropertyName(env, obj, "uiExtensionType", modalUI->uiExtensionType) ||
        !UnwrapStringByPropertyName(env, obj, "uri", modalUI->uri)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "get modal UI extension field fail");
        return false;
    }
    if (!IsExistsByPropertyName(env, obj, "parameters")) {
        return true;
    }
    napi_value params = nullptr;
    napi_get_named_property(env, obj, "parameters", &params);
    napi_valuetype vt = napi_undefined;
    napi_typeof(env, params, &vt);
    if (vt != napi_object) {
        return true;
    }
    auto wp = std::make_shared<AAFwk::WantParams>();
    if (!AppExecFwk::UnwrapWantParams(env, params, *wp)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrap parameters fail");
        return false;
    }
    modalUI->parameters = wp;
    return true;
}

bool UnwrapInteractionInfoOfExecuteResult(
    napi_env env, napi_value param, InsightIntentExecuteResult &executeResult)
{
    napi_value intent = nullptr;
    napi_get_named_property(env, param, "interactionInfo", &intent);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, intent, &valueType);
    if (intent == nullptr || valueType == napi_undefined || valueType == napi_null) {
        return true;
    }
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::JSNAPI, "interactionInfo not object");
        return false;
    }
    napi_value interactionUI = nullptr;
    napi_get_named_property(env, intent, "interactionUI", &interactionUI);
    napi_typeof(env, interactionUI, &valueType);
    if (interactionUI == nullptr || valueType == napi_undefined || valueType == napi_null) {
        return true;
    }
    std::string uiType;
    if (!UnwrapStringByPropertyName(env, interactionUI, "interactionUIType", uiType)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "get interactionUIType fail");
        return false;
    }
    if (uiType == INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
        auto modalUI = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
        modalUI->interactionUIType = uiType;
        if (!UnwrapModalUIExtensionFields(env, interactionUI, modalUI)) {
            return false;
        }
        executeResult.interactionInfo = std::make_shared<AppExecFwk::InteractionInfo>();
        executeResult.interactionInfo->interactionUI = modalUI;
    } else {
        auto ui = std::make_shared<AppExecFwk::InteractionUI>();
        ui->interactionUIType = uiType;
        executeResult.interactionInfo = std::make_shared<AppExecFwk::InteractionInfo>();
        executeResult.interactionInfo->interactionUI = ui;
    }
    if (!InsightIntentExecuteResult::CheckInteractionInfo(*executeResult.interactionInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Check interactionInfo fail");
        return false;
    }
    return true;
}

bool UnwrapOptionalFields(
    napi_env env, napi_value param, InsightIntentExecuteResult &executeResult)
{
    if (IsExistsByPropertyName(env, param, "uris")) {
        std::vector<std::string> uris;
        if (!UnwrapStringArrayByPropertyName(env, param, "uris", uris)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrap uris is null");
            return false;
        }
        executeResult.uris = uris;
    }

    if (IsExistsByPropertyName(env, param, "flags")) {
        int32_t flags = 0;
        if (!UnwrapInt32ByPropertyName(env, param, "flags", flags)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrap flags is null");
            return false;
        }
        executeResult.flags = flags;
    }

    if (IsExistsByPropertyName(env, param, "interactionInfo")) {
        if (!UnwrapInteractionInfoOfExecuteResult(env, param, executeResult)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrap interactionInfo fail");
            return false;
        }
    }
    return true;
}

bool UnwrapExecuteResult(
    napi_env env, napi_value param, InsightIntentExecuteResult &executeResult, bool isDecorator)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");

    if (!IsTypeForNapiValue(env, param, napi_valuetype::napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UnwrapExecuteResult not object");
        return false;
    }
    if (isDecorator) {
        executeResult.isDecorator = true;
        if (!UnwrapResultOfDecoratorExecuteResult(env, param, executeResult)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrap decorator fail");
            return false;
        }
        return true;
    }

    int32_t code = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "code", code)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse code fail");
        return false;
    }
    executeResult.code = code;

    if (IsExistsByPropertyName(env, param, "result")) {
        if (!UnwrapResultOfExecuteResult(env, param, executeResult)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrap result fail");
            return false;
        }
    }

    return UnwrapOptionalFields(env, param, executeResult);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
