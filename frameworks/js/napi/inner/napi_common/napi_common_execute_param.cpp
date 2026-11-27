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

#include "napi_common_execute_param.h"

#include "ability_context.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr const char *CONTEXT_PROPERTY_NAME = "context";

std::shared_ptr<AbilityContext> GetAbilityContextFromJsValue(napi_env env, napi_value contextValue)
{
    void *wrapped = nullptr;
    napi_status ret = napi_unwrap(env, contextValue, &wrapped);
    if (ret != napi_ok || wrapped == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrap ability context failed");
        return nullptr;
    }
    auto weakContext = static_cast<std::weak_ptr<AbilityContext> *>(wrapped);
    return (weakContext != nullptr) ? weakContext->lock() : nullptr;
}

void UnwrapExecuteParamContext(napi_env env, napi_value param,
    AppExecFwk::InsightIntentExecuteParam &executeParam)
{
    if (!IsExistsByPropertyName(env, param, CONTEXT_PROPERTY_NAME)) {
        return;
    }
    napi_value contextValue = nullptr;
    napi_get_named_property(env, param, CONTEXT_PROPERTY_NAME, &contextValue);
    napi_valuetype contextValueType = napi_undefined;
    if (napi_typeof(env, contextValue, &contextValueType) != napi_ok ||
        contextValueType == napi_undefined || contextValueType == napi_null) {
        return;
    }
    auto abilityContext = GetAbilityContextFromJsValue(env, contextValue);
    if (abilityContext == nullptr) {
        TAG_LOGW(AAFwkTag::JSNAPI, "resolve context object failed");
        return;
    }
    executeParam.callerAbilityToken_ = abilityContext->GetToken();
    if (executeParam.callerAbilityToken_ == nullptr) {
        TAG_LOGW(AAFwkTag::JSNAPI, "get caller token from context failed");
    }
}

bool UnwrapRequiredStringParams(napi_env env, napi_value param,
    AppExecFwk::InsightIntentExecuteParam &executeParam)
{
    std::string bundleName {""};
    if (!UnwrapStringByPropertyName(env, param, "bundleName", bundleName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type bundleName");
        return false;
    }
    executeParam.bundleName_ = bundleName;

    std::string moduleName {""};
    if (!UnwrapStringByPropertyName(env, param, "moduleName", moduleName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type moduleName");
        return false;
    }
    executeParam.moduleName_ = moduleName;

    std::string abilityName {""};
    if (!UnwrapStringByPropertyName(env, param, "abilityName", abilityName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type abilityName");
        return false;
    }
    executeParam.abilityName_ = abilityName;

    std::string insightIntentName {""};
    if (!UnwrapStringByPropertyName(env, param, "insightIntentName", insightIntentName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type insightIntentName");
        return false;
    }
    executeParam.insightIntentName_ = insightIntentName;
    return true;
}

bool UnwrapIntentAndTraceParams(napi_env env, napi_value param,
    AppExecFwk::InsightIntentExecuteParam &executeParam)
{
    napi_value napiIntentParam = GetPropertyValueByPropertyName(env, param, "insightIntentParam", napi_object);
    if (napiIntentParam == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null or non-object napiIntentParam");
        return false;
    }

    if (IsExistsByPropertyName(env, param, "deviceId")) {
        TAG_LOGI(AAFwkTag::JSNAPI, "has deviceId");
        std::string deviceId {""};
        if (!UnwrapStringByPropertyName(env, param, "deviceId", deviceId)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type deviceId");
            return false;
        }
        executeParam.deviceId_ = deviceId;
    }

    // toolCallId is an optional parameter: a non-string value is treated as unspecified (no 401 is thrown)
    if (IsExistsByPropertyName(env, param, "toolCallId")) {
        std::string toolCallId {""};
        if (UnwrapStringByPropertyName(env, param, "toolCallId", toolCallId)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "toolCallId %{public}s", toolCallId.c_str());
            executeParam.toolCallId_ = toolCallId;
        } else {
            TAG_LOGW(AAFwkTag::JSNAPI, "toolCallId is not a string, ignored");
        }
    }

    auto wp = std::make_shared<WantParams>();
    if (!AppExecFwk::UnwrapWantParams(env, napiIntentParam, *wp)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrap want fail");
        return false;
    }
    executeParam.insightIntentParam_ = wp;
    return true;
}

bool UnwrapExecuteModeParam(napi_env env, napi_value param,
    AppExecFwk::InsightIntentExecuteParam &executeParam)
{
    int32_t executeMode = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "executeMode", executeMode)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type executeMode");
        return false;
    }
    executeParam.executeMode_ = executeMode;
    return true;
}

bool UnwrapDisplayAndOptionalParams(napi_env env, napi_value param,
    AppExecFwk::InsightIntentExecuteParam &executeParam)
{
    int32_t displayId = INVALID_DISPLAY_ID;
    if (executeParam.executeMode_ == ExecuteMode::UI_ABILITY_FOREGROUND &&
        UnwrapInt32ByPropertyName(env, param, "displayId", displayId)) {
        if (displayId < 0) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid displayId");
            return false;
        }
        TAG_LOGI(AAFwkTag::JSNAPI, "displayId %{public}d", displayId);
        executeParam.displayId_ = displayId;
    }
    if (IsExistsByPropertyName(env, param, "uris")) {
        std::vector<std::string> uris;
        if (!UnwrapStringArrayByPropertyName(env, param, "uris", uris)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument uris fail");
            return false;
        }
        executeParam.uris_ = uris;
    }
    if (IsExistsByPropertyName(env, param, "flags")) {
        int32_t flags = 0;
        if (!UnwrapInt32ByPropertyName(env, param, "flags", flags)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument flags fail");
            return false;
        }
        executeParam.flags_ = flags;
    }
    if (IsExistsByPropertyName(env, param, "userId")) {
        int32_t userId = DEFAULT_INVAL_VALUE;
        if (!UnwrapInt32ByPropertyName(env, param, "userId", userId)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument userId fail");
            return false;
        }
        executeParam.userId_ = userId;
    }
    return true;
}
} // namespace

bool UnwrapExecuteParam(napi_env env, napi_value param, InsightIntentExecuteParam &executeParam)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid params");
        return false;
    }
    if (!UnwrapRequiredStringParams(env, param, executeParam)) {
        return false;
    }
    if (!UnwrapIntentAndTraceParams(env, param, executeParam)) {
        return false;
    }
    if (!UnwrapExecuteModeParam(env, param, executeParam)) {
        return false;
    }
    UnwrapExecuteParamContext(env, param, executeParam);
    return UnwrapDisplayAndOptionalParams(env, param, executeParam);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
