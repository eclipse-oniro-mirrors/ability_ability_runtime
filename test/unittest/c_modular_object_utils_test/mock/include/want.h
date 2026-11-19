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

#ifndef ABILITY_BASE_WANT_H
#define ABILITY_BASE_WANT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AbilityBase_Element {
    char *bundleName;
    char *moduleName;
    char *abilityName;
} AbilityBase_Element;

struct AbilityBase_Want;
typedef struct AbilityBase_Want AbilityBase_Want;

typedef enum {
    ABILITY_BASE_ERROR_CODE_NO_ERROR = 0,
    ABILITY_BASE_ERROR_CODE_PARAM_INVALID = 401,
} AbilityBase_ErrorCode;

#ifdef __cplusplus
}
#endif

// Forward-declare AAFwk::Want for c_modular_object_utils.h
namespace OHOS {
namespace AAFwk {
class Want;
} // namespace AAFwk
} // namespace OHOS

#endif // ABILITY_BASE_WANT_H