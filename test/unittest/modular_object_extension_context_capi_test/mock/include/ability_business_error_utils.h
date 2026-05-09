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

#ifndef MOCK_ABILITY_BUSINESS_ERROR_UTILS_H
#define MOCK_ABILITY_BUSINESS_ERROR_UTILS_H

#include "ability_runtime_common.h"
#include <cstdint>

inline AbilityRuntime_ErrorCode ConvertToCommonBusinessErrorCode(int32_t err)
{
    if (err == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

inline AbilityRuntime_ErrorCode ConvertToAPI17BusinessErrorCode(int32_t err)
{
    if (err == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

#endif // MOCK_ABILITY_BUSINESS_ERROR_UTILS_H
