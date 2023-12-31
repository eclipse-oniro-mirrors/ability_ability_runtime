/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H

#include "ability_business_error.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {

void ThrowError(NativeEngine& engine, int32_t errCode, const std::string& errorMsg = "");
void ThrowError(NativeEngine& engine, const AbilityErrorCode& err);
void ThrowTooFewParametersError(NativeEngine& engine);
void ThrowNoPermissionError(NativeEngine& engine, const std::string& permission);
void ThrowErrorByNativeErr(NativeEngine& engine, int32_t err);

NativeValue* CreateJsError(NativeEngine& engine, const AbilityErrorCode& err);
NativeValue* CreateNoPermissionError(NativeEngine& engine, const std::string& permission);
NativeValue* CreateJsErrorByNativeErr(NativeEngine& engine, int32_t err, const std::string& permission = "");
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H
