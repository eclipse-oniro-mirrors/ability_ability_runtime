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

#ifndef MOCK_ABILITY_RUNTIME_COMMON_H
#define MOCK_ABILITY_RUNTIME_COMMON_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ABILITY_RUNTIME_ERROR_CODE_NO_ERROR = 0,
    ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID = 401,
    ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE = 16000002,
    ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST = 16000011,
    ABILITY_RUNTIME_ERROR_CODE_INTERNAL = 16000050,
} AbilityRuntime_ErrorCode;

typedef struct AbilityRuntime_Context *AbilityRuntime_ContextHandle;

#ifdef __cplusplus
}
#endif

#endif // MOCK_ABILITY_RUNTIME_COMMON_H
