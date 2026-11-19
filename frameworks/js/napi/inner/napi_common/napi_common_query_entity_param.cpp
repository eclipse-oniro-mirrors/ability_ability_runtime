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

#include "napi_common_query_entity_param.h"

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
bool UnwrapQueryEntityWantParamters(napi_env env, napi_value param, InsightIntentQueryParam &queryParam)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid params");
        return false;
    }

    napi_value napiIntentParam = nullptr;
    napi_get_named_property(env, param, "queryEntityParam", &napiIntentParam);
    if (napiIntentParam == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null queryEntityParam");
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiIntentParam, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong argument type queryEntityParam");
        return false;
    }

    std::string queryType = "";
    if (!UnwrapStringByPropertyName(env, napiIntentParam, "queryType", queryType)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type queryType");
        return false;
    }
    if (queryType.compare("byProperty") != 0 && queryType.compare("all") != 0) {
        TAG_LOGE(AAFwkTag::JSNAPI, "queryType must be byProperty or all");
        return false;
    }
    queryParam.queryEntityParam_.queryType_ = queryType;

    napi_value napiParameters = nullptr;
    napi_get_named_property(env, napiIntentParam, "parameters", &napiParameters);
    if (napiParameters == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null parameters");
        return false;
    }

    if (IsExistsByPropertyName(env, napiIntentParam, "parameters")) {
        napi_value napiParameters = nullptr;
        napi_get_named_property(env, napiIntentParam, "parameters", &napiParameters);
        if (napiParameters == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null parameters");
            return false;
        }
        auto wp = std::make_shared<WantParams>();
        if (!AppExecFwk::UnwrapWantParams(env, napiParameters, *wp)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrap want fail");
            return false;
        }
        queryParam.queryEntityParam_.parameters_ = wp;
    }

    return true;
}

bool UnwrapQueryEntityParam(napi_env env, napi_value param, InsightIntentQueryParam &queryParam)
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
    queryParam.bundleName_ = bundleName;

    std::string moduleName {""};
    if (!UnwrapStringByPropertyName(env, param, "moduleName", moduleName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type moduleName");
        return false;
    }
    queryParam.moduleName_ = moduleName;

    std::string intentName {""};
    if (!UnwrapStringByPropertyName(env, param, "intentName", intentName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type intentName");
        return false;
    }
    queryParam.intentName_ = intentName;

    std::string className {""};
    if (!UnwrapStringByPropertyName(env, param, "className", className)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type className");
        return false;
    }
    queryParam.className_ = className;

    if (IsExistsByPropertyName(env, param, "userId")) {
        int32_t userId = DEFAULT_INVAL_VALUE;
        if (!UnwrapInt32ByPropertyName(env, param, "userId", userId)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument userId fail");
        }
        queryParam.userId_ = userId;
    }

    if (!UnwrapQueryEntityWantParamters(env, param, queryParam)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrap paramters fail");
        return false;
    }

    return true;
}

napi_value WrapQueryEntityParam(napi_env env, const std::string &queryType,
    const std::shared_ptr<AAFwk::WantParams> &queryParams)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "WrapQueryEntityParam called queryType:%{public}s", queryType.c_str());
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_create_object failed status:%{public}d", status);
        return nullptr;
    }

    if (queryParams != nullptr) {
        napi_value paramsValue = WrapWantParams(env, *queryParams);
        if (paramsValue == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "WrapWantParams failed");
            return nullptr;
        }
        status = napi_set_named_property(env, object, "parameters", paramsValue);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::JSNAPI, "napi_set_named_property parameters failed status:%{public}d", status);
            return nullptr;
        }
    }

    status = napi_set_named_property(env, object, "queryType", CreateJsValue(env, queryType));
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_set_named_property queryType failed status:%{public}d", status);
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::JSNAPI, "WrapQueryEntityParam success");
    return handleEscape.Escape(object);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
