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

#include "napi_common_skill_execute.h"

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "want_params.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

bool UnwrapSkillExecuteResult(napi_env env, napi_value param, SkillExecuteResult &result)
{
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, param, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::JSNAPI, "param type not object");
        return false;
    }

    // code
    napi_value codeVal = nullptr;
    napi_get_named_property(env, param, "code", &codeVal);
    if (codeVal != nullptr) {
        napi_get_value_int32(env, codeVal, &result.code);
    }

    // result (optional)
    napi_value resultVal = nullptr;
    napi_get_named_property(env, param, "result", &resultVal);
    if (resultVal != nullptr) {
        napi_typeof(env, resultVal, &valueType);
        if (valueType == napi_object) {
            auto wp = std::make_shared<AAFwk::WantParams>();
            if (UnwrapWantParams(env, resultVal, *wp)) {
                result.result = wp;
            }
        }
    }

    // uris (optional)
    napi_value urisVal = nullptr;
    napi_get_named_property(env, param, "uris", &urisVal);
    if (urisVal != nullptr) {
        bool isArray = false;
        napi_is_array(env, urisVal, &isArray);
        if (isArray) {
            uint32_t length = 0;
            napi_get_array_length(env, urisVal, &length);
            for (uint32_t i = 0; i < length; i++) {
                napi_value element = nullptr;
                napi_get_element(env, urisVal, i, &element);
                if (element == nullptr) {
                    continue;
                }
                napi_typeof(env, element, &valueType);
                if (valueType != napi_string) {
                    continue;
                }
                size_t strLen = 0;
                napi_get_value_string_utf8(env, element, nullptr, 0, &strLen);
                std::string uriStr(strLen, '\0');
                napi_get_value_string_utf8(env, element, uriStr.data(), strLen + 1, &strLen);
                result.uris.push_back(uriStr);
            }
        }
    }

    // flags (optional)
    napi_value flagsVal = nullptr;
    napi_get_named_property(env, param, "flags", &flagsVal);
    if (flagsVal != nullptr) {
        napi_typeof(env, flagsVal, &valueType);
        if (valueType == napi_number) {
            uint32_t flags = 0;
            napi_get_value_uint32(env, flagsVal, &flags);
            result.flags = flags;
        }
    }

    return true;
}

napi_value WrapSkillExecuteResult(napi_env env, const SkillExecuteResult &result)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    napi_value codeVal = nullptr;
    napi_create_int32(env, result.code, &codeVal);
    napi_set_named_property(env, obj, "code", codeVal);

    if (result.result != nullptr) {
        napi_value resultVal = WrapWantParams(env, *result.result);
        napi_set_named_property(env, obj, "result", resultVal);
    }

    napi_value urisArray = nullptr;
    napi_create_array(env, &urisArray);
    for (uint32_t i = 0; i < result.uris.size(); i++) {
        napi_value uri = nullptr;
        napi_create_string_utf8(env, result.uris[i].c_str(), result.uris[i].size(), &uri);
        napi_set_element(env, urisArray, i, uri);
    }
    napi_set_named_property(env, obj, "uris", urisArray);

    napi_value flagsVal = nullptr;
    napi_create_uint32(env, result.flags, &flagsVal);
    napi_set_named_property(env, obj, "flags", flagsVal);

    return obj;
}

} // namespace AbilityRuntime
} // namespace OHOS
