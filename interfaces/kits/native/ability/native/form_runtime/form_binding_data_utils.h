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

#ifndef OHOS_ABILITY_RUNTIME_FORM_BINDING_DATA_UTILS_H
#define OHOS_ABILITY_RUNTIME_FORM_BINDING_DATA_UTILS_H

#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {

inline bool GetIsSupportMoreImages(napi_env env, napi_value obj)
{
    napi_value val = nullptr;
    napi_valuetype type = napi_undefined;
    bool result = false;
    if (napi_get_named_property(env, obj, "isSupportMoreImages", &val) == napi_ok && val != nullptr &&
        napi_typeof(env, val, &type) == napi_ok && type == napi_boolean &&
        napi_get_value_bool(env, val, &result) == napi_ok) {
        return result;
    }
    return false;
}

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_FORM_BINDING_DATA_UTILS_H
