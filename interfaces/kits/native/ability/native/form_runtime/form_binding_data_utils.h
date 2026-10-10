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
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

inline bool GetIsSupportMoreImages(napi_env env, napi_value obj)
{
    bool isSupportMoreImages = false;
    napi_value nativeSupportValue = nullptr;
    if (napi_get_named_property(env, obj, "isSupportMoreImages", &nativeSupportValue) == napi_ok &&
        nativeSupportValue != nullptr) {
        napi_valuetype valType = napi_undefined;
        if (napi_typeof(env, nativeSupportValue, &valType) == napi_ok && valType == napi_boolean) {
            if (napi_get_value_bool(env, nativeSupportValue, &isSupportMoreImages) != napi_ok) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to get isSupportMoreImages value");
                isSupportMoreImages = false;
            }
        }
    }
    return isSupportMoreImages;
}

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_FORM_BINDING_DATA_UTILS_H
