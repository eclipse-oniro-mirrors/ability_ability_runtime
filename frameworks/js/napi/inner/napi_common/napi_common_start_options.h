/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_START_OPTIONS_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_START_OPTIONS_H

#include "napi_common_data.h"
#include "start_options.h"

namespace OHOS {
namespace AppExecFwk {
EXTERN_C_START

bool UnwrapStartOptionsWithProcessOption(napi_env env, napi_value param, AAFwk::StartOptions &startOptions);
bool UnwrapStartOptions(napi_env env, napi_value param, AAFwk::StartOptions &startOptions);

bool UnwrapStartOptionsAndWant(napi_env env, napi_value param, AAFwk::StartOptions &startOptions, AAFwk::Want &want);

#ifdef SUPPORT_GRAPHICS
bool UnwrapPixelMapByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, std::shared_ptr<Media::PixelMap> &value);

bool UnwrapPixelMapFromJS(napi_env env, napi_value param, std::shared_ptr<Media::PixelMap> &value);

bool UnwrapStartWindowOption(napi_env env, napi_value param,
    std::shared_ptr<AAFwk::StartWindowOption> &tartWindowOption);

#endif
EXTERN_C_END
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_START_OPTIONS_H
