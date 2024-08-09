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

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
bool UnwrapExecuteParam(napi_env env, napi_value param, InsightIntentExecuteParam &executeParam)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid params");
        return false;
    }

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

    napi_value napiIntentParam = nullptr;
    napi_get_named_property(env, param, "insightIntentParam", &napiIntentParam);
    if (napiIntentParam == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null napiIntentParam");
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiIntentParam, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type intentParam.");
        return false;
    }
    auto wp = std::make_shared<WantParams>();
    if (!AppExecFwk::UnwrapWantParams(env, napiIntentParam, *wp)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrap want fail");
        return false;
    }
    executeParam.insightIntentParam_ = wp;

    int32_t executeMode = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "executeMode", executeMode)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type executeMode");
        return false;
    }
    executeParam.executeMode_ = executeMode;

    int32_t displayId = INVALID_DISPLAY_ID;
    if (executeMode == ExecuteMode::UI_ABILITY_FOREGROUND &&
        UnwrapInt32ByPropertyName(env, param, "displayId", displayId)) {
        if (displayId < 0) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid displayId");
            return false;
        }
        TAG_LOGI(AAFwkTag::JSNAPI, "displayId %{public}d", displayId);
        executeParam.displayId_ = displayId;
    }

    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
