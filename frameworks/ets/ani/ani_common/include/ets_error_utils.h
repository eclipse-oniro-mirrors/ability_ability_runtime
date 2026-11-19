/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ETS_ERROR_UTILS_H
#define OHOS_ABILITY_RUNTIME_ETS_ERROR_UTILS_H

#include "ability_business_error.h"
#include "ani.h"

namespace OHOS {
namespace AbilityRuntime {

class EtsErrorUtil {
public:
    static void ThrowError(ani_env *env, ani_object err);
    static void ThrowError(ani_env *env, int32_t errCode, const std::string &errorMsg = "");
    static void ThrowError(ani_env *env, const AbilityErrorCode &err);
    static void ThrowInvalidCallerError(ani_env *env);
    static void ThrowTooFewParametersError(ani_env *env);
    static void ThrowInvalidNumParametersError(ani_env *env);
    static void ThrowNoPermissionError(ani_env *env, const std::string &permission);
    static void ThrowInvalidParamError(ani_env *env, const std::string &message);
    static void ThrowErrorByNativeErr(ani_env *env, int32_t err);
    static void ThrowNotSystemAppError(ani_env *env);
    static void ThrowEtsTransferClassError(ani_env *env);
    static void ThrowRuntimeError(ani_env *env, int32_t errCode, const std::string &errMessage = "");

    static ani_object CreateError(ani_env *env, const AbilityErrorCode &err);
    static ani_object CreateError(ani_env *env, ani_int code, const std::string &msg);
    static ani_object CreateInvalidParamError(ani_env *env, const std::string &message);
    static ani_object CreateNoPermissionError(ani_env *env, const std::string &permission);
    static ani_object CreateErrorByNativeErr(ani_env *env, int32_t err, const std::string &permission = "",
        const std::string &innerErrMsg = "");
    static ani_object WrapError(ani_env *env, const std::string &msg);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_ERROR_UTILS_H
