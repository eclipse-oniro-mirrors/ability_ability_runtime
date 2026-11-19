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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_ERRORS_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_ERRORS_UTIL_H

#include "ability_manager_errors.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityErrorUtil {

static int32_t ConvertToOriginErrorCode(int32_t errCode)
{
    if (errCode >= ERR_REFINEMENT_INVALID_VALUE_BEGIN && errCode <= ERR_REFINEMENT_INVALID_VALUE_END) {
        if (errCode == ERR_FREQ_START_ABILITY) {
            return errCode;
        }
        return ERR_INVALID_VALUE;
    }
    if (errCode >= ERR_REFINEMENT_INNER_ERROR_BEGIN && errCode <= ERR_REFINEMENT_INNER_ERROR_END) {
        return INNER_ERR;
    }
    if (errCode >= ERR_REFINEMENT_RESOLVE_ABILITY_BEGIN && errCode <= ERR_REFINEMENT_RESOLVE_ABILITY_END) {
        return RESOLVE_ABILITY_ERR;
    }
    if (errCode >= ERR_REFINEMENT_INVALID_CALLER_BEGIN && errCode <= ERR_REFINEMENT_INVALID_CALLER_END) {
        return ERR_INVALID_CALLER;
    }
    return errCode;
}

} // AbilityErrorUtil
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_ERRORS_UTIL_H
