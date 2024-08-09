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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_MANAGER_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_MANAGER_UTILS_H

#include "auto_startup_info.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace AbilityRuntime {
bool UnwrapAutoStartupInfo(napi_env env, napi_value param, AutoStartupInfo &info);
bool IsNormalObject(napi_env env, napi_value value);
napi_value CreateJsAutoStartupInfoArray(napi_env env, const std::vector<AutoStartupInfo> &infoList);
napi_value CreateJsAutoStartupInfo(napi_env env, const AutoStartupInfo &info);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_MANAGER_UTILS_H